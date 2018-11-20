extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "tools/lua_helpers.h"
#include "serialize.h"
#include "lua_thread.h"
#include "common/base_lambda_support.h"
#include "common/business_runtime.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread_restrictions.h"

static int unpack(lua_State *L);
static int postToThread(lua_State *L);
static int postToThreadSync(lua_State *L);
static int createThread(lua_State *L);
static int synchronized(lua_State *L);
static int currentThread(lua_State *L);
static int getThreadByName(lua_State *L);
static int threadCount(lua_State *L);
static int callbackGc(lua_State *L);
static int callbackCall(lua_State *L);

static std::recursive_mutex  globle_lock;
static std::recursive_mutex  createThread_lock;

static const struct luaL_Reg metaFunctions[] = {
    {NULL, NULL}
};

static const struct luaL_Reg functions[] = {
    {"threadCount", threadCount},
    {"currentThread", currentThread},
    {"getThreadByName", getThreadByName},
    {"createThread", createThread},
    {"postToThread", postToThread},
    {"postToThreadSync", postToThreadSync},
    {"synchronized", synchronized},
    {"unpack", unpack},
    {NULL, NULL}
};

static int unpack(lua_State *L){
    return seri_unpack(L);
}

static int synchronized(lua_State *L){
    std::lock_guard<std::recursive_mutex> guard(globle_lock);
    BEGIN_STACK_MODIFY(L)
    if(lua_isfunction(L, -1)) {
        int err = lua_pcall(L, 0, 0, 0);
        if (err != 0) {
            luaL_error(L,"synchronized call error");
        }
    } else {
        luaL_error(L, "can only synchronized lua function");
    }
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int getThreadByName(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    std::string name = luaL_checkstring(L, -1);
    BusinessThreadID thread_identifier;
    bool hasThread = BusinessThread::GetThreadIdentifierByName(&thread_identifier,name);
    if (hasThread) {
        lua_pushinteger(L, thread_identifier);
    } else {
        lua_pushinteger(L, -1);
    }
    END_STACK_MODIFY(L, 1)
    return 1;
}

static int currentThread(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    BusinessThreadID now_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
    lua_pushinteger(L, now_thread_identifier);
    END_STACK_MODIFY(L, 1)
    return 1;
}

static int threadCount(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    int count = BusinessThread::getThreadCount();
    lua_pushinteger(L, count);
    END_STACK_MODIFY(L, 1)
    return 1;
}

static int createThread(lua_State *L)
{
    std::lock_guard<std::recursive_mutex> guard(createThread_lock);
    BEGIN_STACK_MODIFY(L)
    BusinessThreadID now_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
    const char * name = luaL_checkstring(L, -1);
    int type = luaL_checkint(L, -2);
    BusinessThreadID new_identifier;
    bool hasThread = BusinessThread::GetThreadIdentifierByName(&new_identifier, name);
    if(hasThread) {
        lua_pushinteger(L, new_identifier);
    } else {
        int newThreadId = BusinessRuntime::GetRuntime()->createNewThread((BusinessThread::ID)type, name);
        lua_pushinteger(L, newThreadId);
    }
    END_STACK_MODIFY(L, 1)
    return 1;
}


static int postToThreadSync(lua_State *L){
    BEGIN_STACK_MODIFY(L)
    int toThread = luaL_checknumber(L, 1);
    BusinessThreadID from_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&from_thread_identifier);
    lua_remove(L, 1);
    std::string moduleName = luaL_checkstring(L, 1);
    //只允许数据库模型内部使用，业务逻辑使用要注意死锁的问题，这里不开放给普通业务
//    DCHECK(moduleName == "DB" || moduleName == "DBCache");
    lua_remove(L, 1);
    std::string methodName = luaL_checkstring(L, 1);
    lua_remove(L, 1);
    block * params = NULL;
    //        int top = lua_gettop(L);
    std::list<thread::CallbackContext *> callbackContexts;
    if(lua_gettop(L)>0){
        params = seri_pack(L,callbackContexts);
    }
    int *countPtr = new int(0);
    if(toThread == from_thread_identifier) {
        //同线程
        std::string lua;
        lua_State * state = BusinessThread::GetCurrentThreadLuaState();
        if (params != NULL) {
            lua_pushlightuserdata(state, params);
            lua_setglobal(state, "cParams");
            lua = "LUA_SYNC_RESULT = {require('"+moduleName+"')." + methodName +"(lua_thread.unpack(cParams))}";
        } else {
            lua = "LUA_SYNC_RESULT = {require('"+moduleName+"')." + methodName +"()}";
        }
        luaL_dostring(state, lua.c_str());
        if (params != NULL) {
            lua_pushnil(state);
            lua_setglobal(state, "cParams");
        }
        lua_getglobal(state, "LUA_SYNC_RESULT");
        lua_pushnil(state);
        lua_setglobal(state, "LUA_SYNC_RESULT");
        
        if (!lua_isnil(state, -1) && lua_istable(state, -1)) {
            int paramsCount = 0;
            lua_pushnil(state);
            int lastIndex = 1;
            while (lua_next(state, -2)) {
                int index = lua_tonumber(state, -2);
                while(lastIndex<index) {
                    lua_pushnil(state);
                    lua_insert(state, -4);
                    paramsCount++;
                    lastIndex++;
                }
                lua_insert(state, -3);
                paramsCount++;
                lastIndex++;
            }
            lua_pop(state, 1);
            *countPtr = paramsCount;
        } else {
            lua_pop(state, 1);
        }
    } else {
//        top = lua_gettop(L);
        //记录所有的thread::CallbackContext 的toTheadList 字段
        std::list<thread::CallbackContext *>::iterator it;
        bool hasAddParamToStrongTable = false;
        for(it=callbackContexts.begin();it!=callbackContexts.end();it++)
        {
            thread::CallbackContext * callback = *it;
            if(callback->fromThread != toThread){
                std::lock_guard<std::recursive_mutex> guard(callback->lock);
                thread::CallbackInfo info;
                info.threadId = from_thread_identifier;
                info.fromMethod = methodName;
                info.fromModule = moduleName;
                callback->toTheadList[(BusinessThreadID)toThread] = info;
            }
            pushUserdataInWeakTable(L, callback);
            if (!lua_isnil(L, -1)) {
                //存储在strongtable["postToThread"] = array ; array[toThread] = params ; params[i] = param;param[thread::CallbackContext *] = userdata
//                top = lua_gettop(L);
                pushStrongUserdataTable(L);
                lua_pushstring(L, "postToThread");
                lua_rawget(L, -2);
                if (lua_isnil(L, -1)) {
                    lua_pop(L, 1);
                    lua_pushstring(L, "postToThread");
                    lua_newtable(L);
                    lua_rawset(L, -3);
                    lua_pushstring(L, "postToThread");
                    lua_rawget(L, -2);
                }
//                top = lua_gettop(L);
                lua_remove(L, -2);
                lua_pushinteger(L, toThread);
                lua_rawget(L, -2);
                if (lua_isnil(L, -1)) {//queue为空，初始化
                    lua_pop(L, 1);
                    lua_pushinteger(L, toThread);
                    lua_newtable(L);
                    lua_pushstring(L, "start");
                    lua_pushinteger(L, 0);
                    lua_rawset(L, -3);
                    lua_pushstring(L, "end");
                    lua_pushinteger(L, 0);
                    lua_rawset(L, -3);
                    lua_rawset(L, -3);
                    lua_pushinteger(L, toThread);
                    lua_rawget(L, -2);
                }
//                top = lua_gettop(L);
                lua_remove(L, -2);
                lua_pushstring(L, "end");
                lua_rawget(L, -2);
                long n = lua_tointeger(L, -1);
                lua_pop(L, 1);
                lua_pushinteger(L, n+1);
                lua_rawget(L, -2);
//                top = lua_gettop(L);
                if (lua_isnil(L, -1)) {
                    lua_pop(L, 1);
                    lua_pushinteger(L, n+1);
                    lua_newtable(L);
                    lua_rawset(L, -3);
                    lua_pushinteger(L, n+1);
                    lua_rawget(L, -2);
                }
                lua_pushlightuserdata(L, callback);
                lua_pushvalue(L, -3);
                lua_rawset(L, -3);
                lua_pop(L, 1);
                hasAddParamToStrongTable = true;
            }
            lua_pop(L, 1);
        }
        
        if (hasAddParamToStrongTable) {
            //        queue的end+1
            pushStrongUserdataTable(L);
            lua_pushstring(L, "postToThread");
            lua_rawget(L, -2);
            lua_pushinteger(L, toThread);
            lua_rawget(L, -2);
            lua_pushstring(L, "end");
            lua_rawget(L, -2);
            long n = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_pushstring(L, "end");
            lua_pushinteger(L, n+1);
            lua_rawset(L, -3);
            lua_pop(L, 3);
        }
        base::WaitableEvent * event = new base::WaitableEvent(false,false);
        base::ThreadRestrictions::ScopedAllowWait allow_wait;
        
        block ** resultParams = new block*() ;
        
        //所有callbackContexts也暂时不能回收，等真正调用完才能回收，从weak表copy到strong表
        BusinessThread::PostTask((BusinessThreadID)toThread, FROM_HERE, base::BindLambda([=](){
            std::string lua;
            lua_State * state = BusinessThread::GetCurrentThreadLuaState();
            if (params != NULL) {
                lua_pushlightuserdata(state, params);
                lua_setglobal(state, "cParams");
                lua = "LUA_SYNC_RESULT = {require('"+moduleName+"')." + methodName +"(lua_thread.unpack(cParams))}";
            } else {
                lua = "LUA_SYNC_RESULT = {require('"+moduleName+"')." + methodName +"()}";
            }
            luaL_dostring(state, lua.c_str());
            if (params != NULL) {
                lua_pushnil(state);
                lua_setglobal(state, "cParams");
            }
            lua_getglobal(state, "LUA_SYNC_RESULT");
            lua_pushnil(state);
            lua_setglobal(state, "LUA_SYNC_RESULT");
            if (!lua_isnil(state, -1) && lua_istable(state, -1)) {
                int paramsCount = 0;
                lua_pushnil(state);
                int lastIndex = 1;
                while (lua_next(state, -2)) {
                    int index = lua_tonumber(state, -2);
                    while(lastIndex<index) {
                        lua_pushnil(state);
                        lua_insert(state, -4);
                        paramsCount++;
                        lastIndex++;
                    }
                    lua_insert(state, -3);
                    paramsCount++;
                    lastIndex++;
                }
                lua_pop(state, 1);
                std::list<thread::CallbackContext *> resultCallbackContext;
                *resultParams = seri_pack(state,resultCallbackContext,paramsCount);
                *countPtr = paramsCount;
                if (resultCallbackContext.size()>0) {
                    luaL_error(state, "can not pass callback in sync result");
                }
                lua_pop(state, paramsCount);
            } else {
                lua_pop(state, 1);
            }
            lua_gc(state, LUA_GCCOLLECT, 0);
            event->Signal();
            if (hasAddParamToStrongTable) {
                BusinessThread::PostTask(from_thread_identifier, FROM_HERE, base::BindLambda([=](){
                    //把param 从强表移除
                    lua_State * fromState = BusinessThread::GetCurrentThreadLuaState();
                    pushStrongUserdataTable(fromState);
                    lua_pushstring(L, "postToThread");
                    lua_rawget(fromState, -2);
                    lua_pushinteger(fromState, toThread);
                    lua_rawget(fromState, -2);
                    lua_pushstring(fromState, "start");
                    lua_rawget(fromState, -2);
                    long start = lua_tointeger(fromState, -1);
                    lua_pop(fromState, 1);
                    lua_pushinteger(fromState, start + 1);
                    lua_pushnil(fromState);
                    lua_rawset(fromState, -3);
                    lua_pushstring(fromState, "start");
                    lua_pushinteger(fromState, start + 1);
                    lua_rawset(fromState, -3);
                    lua_pushstring(fromState, "end");
                    lua_rawget(fromState, -2);
                    long end = lua_tointeger(fromState, -1);
                    lua_pop(fromState, 1);
                    if (start + 1 == end) {
                        //postToThread[toThread] == nil
                        lua_pop(fromState, 1);
                        lua_pushinteger(fromState, toThread);
                        lua_pushnil(fromState);
                        lua_rawset(fromState, -3);
                        lua_pop(fromState, 2);
                    } else {
                        lua_pop(fromState, 3);
                    }
                    lua_gc(fromState, LUA_GCCOLLECT, 0);
                    BusinessThread::PostTask((BusinessThreadID)toThread, FROM_HERE, base::BindLambda([=](){
                        lua_State * s = BusinessThread::GetCurrentThreadLuaState();
                        lua_gc(s, LUA_GCCOLLECT, 0);
                    }));
                }));
            }
        }));
        event->Wait();
        delete event;
        while (lua_gettop(L)>0) {
            lua_pop(L, 1);
        }
        lua_pushlightuserdata(L, *resultParams);
        unpack(L);
        delete resultParams;
    }
    int c = *countPtr;
    delete countPtr;
    END_STACK_MODIFY(L, c)
    return c;
}

static int postToThread(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    int toThread = luaL_checknumber(L, 1);
    lua_remove(L, 1);
    std::string moduleName = luaL_checkstring(L, 1);
    lua_remove(L, 1);
    std::string methodName = luaL_checkstring(L, 1);
    lua_remove(L, 1);
    
    block * params = NULL;
//    int top = lua_gettop(L);
    std::list<thread::CallbackContext *> callbackContexts;
    if (lua_gettop(L) > 0) {
        params = seri_pack(L,callbackContexts);
    }
//    top = lua_gettop(L);
    BusinessThreadID from_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&from_thread_identifier);
    //记录所有的thread::CallbackContext 的toTheadList 字段
    std::list<thread::CallbackContext *>::iterator it;
    bool hasAddParamToStrongTable = false;
    for(it=callbackContexts.begin();it!=callbackContexts.end();it++)
    {
        thread::CallbackContext * callback = *it;
        if(callback->fromThread != toThread){
            std::lock_guard<std::recursive_mutex> guard(callback->lock);
            thread::CallbackInfo info;
            info.threadId = from_thread_identifier;
            info.fromMethod = methodName;
            info.fromModule = moduleName;
            callback->toTheadList[(BusinessThreadID)toThread] = info;
        }
        pushUserdataInWeakTable(L, callback);
        if (!lua_isnil(L, -1)) {
            //存储在strongtable["postToThread"] = array ; array[toThread] = params ; params[i] = param;param[thread::CallbackContext *] = userdata
//            top = lua_gettop(L);
            pushStrongUserdataTable(L);
            lua_pushstring(L, "postToThread");
            lua_rawget(L, -2);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                lua_pushstring(L, "postToThread");
                lua_newtable(L);
                lua_rawset(L, -3);
                lua_pushstring(L, "postToThread");
                lua_rawget(L, -2);
            }
//            top = lua_gettop(L);
            lua_remove(L, -2);
            lua_pushinteger(L, toThread);
            lua_rawget(L, -2);
            if (lua_isnil(L, -1)) {//queue为空，初始化
                lua_pop(L, 1);
                lua_pushinteger(L, toThread);
                lua_newtable(L);
                lua_pushstring(L, "start");
                lua_pushinteger(L, 0);
                lua_rawset(L, -3);
                lua_pushstring(L, "end");
                lua_pushinteger(L, 0);
                lua_rawset(L, -3);
                lua_rawset(L, -3);
                lua_pushinteger(L, toThread);
                lua_rawget(L, -2);
            }
//            top = lua_gettop(L);
            lua_remove(L, -2);
            lua_pushstring(L, "end");
            lua_rawget(L, -2);
            long n = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_pushinteger(L, n+1);
            lua_rawget(L, -2);
//            top = lua_gettop(L);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                lua_pushinteger(L, n+1);
                lua_newtable(L);
                lua_rawset(L, -3);
                lua_pushinteger(L, n+1);
                lua_rawget(L, -2);
            }
            lua_pushlightuserdata(L, callback);
            lua_pushvalue(L, -3);
            lua_rawset(L, -3);
            lua_pop(L, 1);
            hasAddParamToStrongTable = true;
        }
        lua_pop(L, 1);
    }
    
    if (hasAddParamToStrongTable) {
//        queue的end+1
        pushStrongUserdataTable(L);
        lua_pushstring(L, "postToThread");
        lua_rawget(L, -2);
        lua_pushinteger(L, toThread);
        lua_rawget(L, -2);
        lua_pushstring(L, "end");
        lua_rawget(L, -2);
        long n = lua_tointeger(L, -1);
        lua_pop(L, 1);
        lua_pushstring(L, "end");
        lua_pushinteger(L, n+1);
        lua_rawset(L, -3);
        lua_pop(L, 3);
    }
    
    //所有callbackContexts也暂时不能回收，等真正调用完才能回收，从weak表copy到strong表
    
    BusinessThread::PostTask((BusinessThreadID)toThread, FROM_HERE, base::BindLambda([=](){
        std::string lua;
        lua_State * state = BusinessThread::GetCurrentThreadLuaState();
        if (params != NULL) {
            lua_pushlightuserdata(state, params);
            lua_setglobal(state, "cParams");
            lua = "require('"+moduleName+"')." + methodName +"(lua_thread.unpack(cParams))";
        } else {
            lua = "require('"+moduleName+"')." + methodName +"()";
        }
        luaL_dostring(state, lua.c_str());
        if (params != NULL) {
            lua_pushnil(state);
            lua_setglobal(state, "cParams");
        }
        lua_gc(state, LUA_GCCOLLECT, 0);
        if (hasAddParamToStrongTable) {
            BusinessThread::PostTask(from_thread_identifier, FROM_HERE, base::BindLambda([=](){
                //把param 从强表移除
                lua_State * fromState = BusinessThread::GetCurrentThreadLuaState();
                pushStrongUserdataTable(fromState);
                lua_pushstring(L, "postToThread");
                lua_rawget(fromState, -2);
                lua_pushinteger(fromState, toThread);
                lua_rawget(fromState, -2);
                lua_pushstring(fromState, "start");
                lua_rawget(fromState, -2);
                long start = lua_tointeger(fromState, -1);
                lua_pop(fromState, 1);
                lua_pushinteger(fromState, start + 1);
                lua_pushnil(fromState);
                lua_rawset(fromState, -3);
                lua_pushstring(fromState, "start");
                lua_pushinteger(fromState, start + 1);
                lua_rawset(fromState, -3);
                lua_pushstring(fromState, "end");
                lua_rawget(fromState, -2);
                long end = lua_tointeger(fromState, -1);
                lua_pop(fromState, 1);
                if (start + 1 == end) {
                    //postToThread[toThread] == nil
                    lua_pop(fromState, 1);
                    lua_pushinteger(fromState, toThread);
                    lua_pushnil(fromState);
                    lua_rawset(fromState, -3);
                    lua_pop(fromState, 2);
                } else {
                    lua_pop(fromState, 3);
                }
                lua_gc(fromState, LUA_GCCOLLECT, 0);
                BusinessThread::PostTask((BusinessThreadID)toThread, FROM_HERE, base::BindLambda([=](){
                     lua_State * s = BusinessThread::GetCurrentThreadLuaState();
                     lua_gc(s, LUA_GCCOLLECT, 0);
                }));
            }));
        }
    }));
    END_STACK_MODIFY(L, 0)
    return 0;
}

extern int luaopen_thread(lua_State *L) {
    BEGIN_STACK_MODIFY(L);
    luaL_newmetatable(L, LUA_THREAD_METATABLE_NAME);
    luaL_register(L, NULL, metaFunctions);
    luaL_register(L, LUA_THREAD_METATABLE_NAME, functions);
    lua_pushvalue(L, -2);
    lua_setmetatable(L, -2); // Set the metatable for the module
    
    lua_pushnumber(L, BusinessThread::UI);
    lua_setglobal(L, "BusinessThreadUI");
    lua_pushnumber(L, BusinessThread::DB);
    lua_setglobal(L, "BusinessThreadDB");
    lua_pushnumber(L, BusinessThread::LOGIC);
    lua_setglobal(L, "BusinessThreadLOGIC");
    lua_pushnumber(L, BusinessThread::FILE);
    lua_setglobal(L, "BusinessThreadFILE");
    lua_pushnumber(L, BusinessThread::IO);
    lua_setglobal(L, "BusinessThreadIO");
    
    END_STACK_MODIFY(L, 0)
    return 0;
}
static const struct luaL_Reg callBackMetaFunctions[] = {
    {"__gc", callbackGc},
    {"__call", callbackCall},
    {NULL, NULL}
};

static const struct luaL_Reg callBackFunctions[] = {
    {NULL, NULL}
};

static int callbackGc(lua_State *L) {
    thread::CallbackContext **instanceUserdata = (thread::CallbackContext **)luaL_checkudata(L, 1, LUA_CALLBACK_METATABLE_NAME);
    BusinessThreadID now_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
    thread::CallbackContext * localInstanceUserdata = *instanceUserdata;
    std::lock_guard<std::recursive_mutex> guard(localInstanceUserdata->lock);
    thread::ITER it = localInstanceUserdata->toTheadList.find(now_thread_identifier);
    if(it != localInstanceUserdata->toTheadList.end()){
        localInstanceUserdata->toTheadList.erase(it);
    }
//    printf("gc %d %d\n",now_thread_identifier,localInstanceUserdata);
    if (localInstanceUserdata->toTheadList.size() == 0) {
        //释放thread::CallbackContext，并在strong 表去除function
        BusinessThreadID from_thread_identifier = localInstanceUserdata->fromThread;
        BusinessThread::PostTask(from_thread_identifier, FROM_HERE, base::BindLambda([=](){
//            printf("real gc %d %d\n",now_thread_identifier,localInstanceUserdata);
            //切线程以后再检查是否可以清除function
            bool toTheadListEmpty = true;
            {
                std::lock_guard<std::recursive_mutex> guard2(localInstanceUserdata->lock);
                toTheadListEmpty = localInstanceUserdata->toTheadList.size() == 0;
            }
            if(toTheadListEmpty){
                //获取对用的lua_State
                lua_State * fromState = BusinessThread::GetCurrentThreadLuaState();
                pushStrongUserdataTable(fromState);
                lua_pushlightuserdata(fromState, localInstanceUserdata);
                lua_pushnil(fromState);
                lua_rawset(fromState, -3);
                lua_pop(fromState, 1);
                delete localInstanceUserdata;
            }
        }));
    }
    return 0;
}

static int callbackCall(lua_State *L) {
    BEGIN_STACK_MODIFY(L);
    thread::CallbackContext** callbackContext = (thread::CallbackContext **)luaL_checkudata(L, 1, LUA_CALLBACK_METATABLE_NAME);
    BusinessThreadID now_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
    BusinessThreadID from_thread_identifier = (*callbackContext)->fromThread;
    thread::CallbackContext* localCallbackContext = *callbackContext;
    
    if (now_thread_identifier != from_thread_identifier) {
        //StrongTable[localCallbackContext] ->[params]多个调用的参数  params -> [param(thread::CallbackContext*:thread::CallbackContext**userdata)]单个调用的参数
        //函数调用过程中，thread::CallbackContext**的userdata加入强表，避免被回收
        pushUserdataInStrongTable(L, localCallbackContext);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            pushStrongUserdataTable(L);
            lua_pushlightuserdata(L, localCallbackContext);
            lua_newtable(L);
            lua_rawset(L, -3);
            lua_pushlightuserdata(L, localCallbackContext);
            lua_rawget(L, -2);
            lua_remove(L, -2);
            //构造循环队列table
            lua_pushstring(L, "start");
            lua_pushinteger(L, 0);
            lua_rawset(L, -3);
            lua_pushstring(L, "end");
            lua_pushinteger(L, 0);
            lua_rawset(L, -3);
        }//top new table
        lua_pushstring(L, "end");
        lua_rawget(L, -2);
        long n = lua_tointeger(L, -1);
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushlightuserdata(L, localCallbackContext);
        lua_pushvalue(L, 1);
        lua_rawset(L, -3);
        lua_pushinteger(L, n+1);
        lua_pushvalue(L, -2);
        lua_remove(L, -3);
        lua_rawset(L, -3);
        lua_pushstring(L, "end");
        lua_pushinteger(L, n+1);
        lua_rawset(L, -3);
        lua_pop(L, 1);
    }
    lua_remove(L,1);//pop掉userdata
    int paramsNum = lua_gettop(L);
    block * params = NULL;
    std::list<thread::CallbackContext *> callbackContexts;
    params = seri_pack(L,callbackContexts);
    //记录所有的thread::CallbackContext 的toTheadList 字段
    std::list<thread::CallbackContext *>::iterator it;
    for(it=callbackContexts.begin();it!=callbackContexts.end();it++)
    {
        thread::CallbackContext * callback = *it;
        if(callback->fromThread != from_thread_identifier){
            std::lock_guard<std::recursive_mutex> guard(callback->lock);
            thread::CallbackInfo info;
            info.threadId = now_thread_identifier;
            callback->toTheadList[(BusinessThreadID)from_thread_identifier] = info;
        }
    }
    
    if (now_thread_identifier == from_thread_identifier) {
        //同线程
        pushStrongUserdataTable(L);
        lua_pushlightuserdata(L, localCallbackContext);
        lua_rawget(L, -2);
        lua_remove(L, -2);//盏顶是function
        if (lua_isfunction(L, -1)) {
            lua_pushcfunction(L, seri_unpack);
            lua_pushlightuserdata(L, params);
            int err = lua_pcall(L, 1, paramsNum, 0);
            
            if (err != 0) {
                luaL_error(L,"seri_unpack call error");
            } else {
                err = lua_pcall(L, paramsNum, 0, 0);
                if (err != 0) {
                    luaL_error(L,"call error %s", lua_tostring(L, -1));
                }
            }
        } else {
            lua_pop(L, 1);
        }
    } else {
        //所有callbackContexts也暂时不能回收，等真正调用完才能回收，从weak表copy到strong表
        for(it=callbackContexts.begin();it!=callbackContexts.end();it++)
        {
            thread::CallbackContext * callback = *it;
            pushUserdataInWeakTable(L, callback);
            if (!lua_isnil(L, -1)) {
                pushUserdataInStrongTable(L, localCallbackContext);
                if (lua_isnil(L, -1)) {
                    luaL_error(L, "no param table");
                    lua_pop(L, 1);
                    lua_pop(L, 1);
                } else {
                    lua_pushstring(L, "end");
                    lua_rawget(L, -2);
                    long n = lua_tointeger(L, -1);
                    lua_pop(L, 1);
                    lua_pushinteger(L, n);
                    lua_rawget(L, -2);
                    lua_pushlightuserdata(L, callback); //top thread::CallbackContext* table table userdata
                    lua_pushvalue(L, -4);//top userdata thread::CallbackContext* table talbe userdata
                    lua_rawset(L, -3); //table talbe userdata
                    lua_pop(L, 3);
                }
            } else {
                lua_pop(L, 1);
            }
        }
        
        BusinessThread::PostTask(from_thread_identifier, FROM_HERE, base::BindLambda([=](){
            lua_State * fromState = BusinessThread::GetCurrentThreadLuaState();
            pushStrongUserdataTable(fromState);
            lua_pushlightuserdata(fromState, localCallbackContext);
            lua_rawget(fromState, -2);
            lua_remove(fromState, -2);//盏顶是function
            if (lua_isfunction(fromState, -1)) {
                lua_pushcfunction(fromState, seri_unpack);
                lua_pushlightuserdata(fromState, params);
                int err = lua_pcall(fromState, 1, paramsNum, 0);
                
                if (err != 0) {
                    luaL_error(fromState,"seri_unpack error");
                } else {
                    err = lua_pcall(fromState, paramsNum, 0, 0);
                    if (err != 0) {
                        luaL_error(fromState, "call error %s",lua_tostring(L, -1));
                    }
                }
            } else {
                lua_pop(fromState, 1);
            }
            //函数调用完清掉强表引用
            BusinessThread::PostTask(now_thread_identifier, FROM_HERE, base::BindLambda([=](){
                lua_State * nowState = BusinessThread::GetCurrentThreadLuaState();
                pushUserdataInStrongTable(L, localCallbackContext);
                if (lua_isnil(nowState, -1)) {
                    luaL_error(nowState, "no param table when finish");
                    lua_pop(nowState, 1);
                } else {
                    lua_pushstring(nowState, "start");
                    lua_rawget(nowState, -2);
                    long start = lua_tointeger(nowState, -1);
                    lua_pop(nowState, 1);
                    
                    lua_pushstring(nowState, "end");
                    lua_rawget(nowState, -2);
                    long end = lua_tointeger(nowState, -1);
                    lua_pop(nowState, 1);
                    
                    if (end == start) {
                        luaL_error(nowState, " param table empty when finish");
                        lua_pop(nowState, 1);
                    } else {
                        lua_pushinteger(nowState, start + 1);
                        lua_pushnil(nowState);
                        lua_rawset(nowState, -3);
                        
                        lua_pushstring(nowState, "start");
                        lua_pushinteger(nowState, start + 1);
                        lua_rawset(nowState, -3);
                        lua_pop(nowState, 1);
                    }
                    
                    if(start + 1 == end){
                        //队列里面没东西了，清空
                        pushStrongUserdataTable(nowState);
                        lua_pushlightuserdata(nowState, localCallbackContext);
                        lua_pushnil(nowState);
                        lua_rawset(nowState, -3);
                        lua_pop(nowState, 1);
                    }
                }
                lua_gc(nowState, LUA_GCCOLLECT, 0);
                BusinessThread::PostTask(from_thread_identifier, FROM_HERE, base::BindLambda([=](){
                    lua_State * S = BusinessThread::GetCurrentThreadLuaState();
                    lua_gc(S, LUA_GCCOLLECT, 0);
                }));
            }));
        }));
    }
    END_STACK_MODIFY(L, 0)
    return 0;
}

extern int luaopen_callback(lua_State* L)
{
    BEGIN_STACK_MODIFY(L);
    luaL_newmetatable(L, LUA_CALLBACK_METATABLE_NAME);
    luaL_register(L, NULL, callBackMetaFunctions);
    luaL_register(L, LUA_CALLBACK_METATABLE_NAME, callBackFunctions);
    END_STACK_MODIFY(L, 0)
    return 0;
}
