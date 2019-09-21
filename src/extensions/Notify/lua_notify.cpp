extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "tools/lua_helpers.h"
#include "lua_notify.h"
#include "common/notification_service.h"
#include "common/base_lambda_support.h"
#include "serialize.h"

static int postNotification(lua_State *L);
static int createListener(lua_State *L);
static int AddObserver(lua_State *L);
static int RemoveObserver(lua_State *L);
static int __index(lua_State *L);
static int __newindex(lua_State *L);
static int __gc(lua_State *L);

static int postNotification(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    lua_assert(lua_gettop(L) == 1 ||  lua_gettop(L) == 2);
    int type = luaL_checkint(L, 1);
    lua_remove(L, 1);
    BusinessThreadID now_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
    if (now_thread_identifier == BusinessThread::UI) {
        Notification::sourceType source = Notification::LUA;
        content::NotificationService::current()->Notify(type,content::Source<Notification::sourceType>(&source),content::Details<lua_State>(L));
    } else {
        block * params = NULL;
        if(lua_gettop(L) == 1){
            std::list<thread::CallbackContext *> callbackContexts;
            params = seri_pack(L,callbackContexts);
        }
        BusinessThread::PostTask(BusinessThread::UI, FROM_HERE, base::BindLambda([=](){
            lua_State * state = BusinessThread::GetCurrentThreadLuaState();
            if (params) {
                lua_pushcfunction(state, seri_unpack);
                lua_pushlightuserdata(state, params);
                int err = lua_pcall(state, 1, 1, 0);
                if (err != 0) {
                    luaL_error(state,"postNotification seri_unpack call error");
                }
            } else {
                lua_pushnil(state);
            }
            Notification::sourceType source = Notification::LUA;
            content::NotificationService::current()->Notify(type,content::Source<Notification::sourceType>(&source),content::Details<lua_State>(state));
            lua_pop(state, 1);
        }));
    }
    END_STACK_MODIFY(L, 0)
    return 0;
}


static int AddObserver(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    listenerWrapper **instanceUserdata = (listenerWrapper **)luaL_checkudata(L, 1, LUA_NOTIFICATION_METATABLE_NAME);
    lua_Integer type = luaL_checkinteger(L, 2);
    lua_settable(L, -3);
    listenerWrapper * pointer = *instanceUserdata;
    BusinessThreadID now_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
    if (now_thread_identifier == BusinessThread::UI) {
        pointer->listener->AddObserver((int)type);
    } else {
        BusinessThread::PostTask(BusinessThread::UI, FROM_HERE, base::BindLambda([=](){
            pointer->listener->AddObserver((int)type);
        }));
    }
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int RemoveObserver(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    listenerWrapper **instanceUserdata = (listenerWrapper **)luaL_checkudata(L, 1, LUA_NOTIFICATION_METATABLE_NAME);
    lua_Integer type = luaL_checkinteger(L, 2);
    lua_pushnil(L);
    lua_settable(L, -3);
    listenerWrapper * pointer = *instanceUserdata;
    BusinessThreadID now_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
    if (now_thread_identifier == BusinessThread::UI) {
        pointer->listener->RemoveObserver((int)type);
    } else {
        BusinessThread::PostTask(BusinessThread::UI, FROM_HERE, base::BindLambda([=](){
            pointer->listener->RemoveObserver((int)type);
        }));
    }
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int createListener(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    BusinessThreadID now_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
    if (now_thread_identifier == BusinessThread::UI) {
        scoped_refptr<LuaNotificationListener> l = make_scoped_refptr(new LuaNotificationListener());
        listenerWrapper ** listener = new (listenerWrapper *);
        *listener = new listenerWrapper();
        (*listener)->listener = l;
        size_t nbytes = sizeof(listenerWrapper **);
        listenerWrapper ** instanceUserdata = (listenerWrapper **)lua_newuserdata(L, nbytes);
        *instanceUserdata = *listener;
        luaL_getmetatable(L, LUA_NOTIFICATION_METATABLE_NAME);
        lua_setmetatable(L, -2);
        
        lua_newtable(L);
        lua_pushcfunction(L, AddObserver);
        lua_setfield(L, -2, "AddObserver");
        lua_pushcfunction(L, RemoveObserver);
        lua_setfield(L, -2, "RemoveObserver");
        lua_setfenv(L, -2);
        
        pushWeakUserdataTable(L);
        lua_pushlightuserdata(L, l.get());
        lua_pushvalue(L, -3);
        lua_rawset(L, -3);
        lua_pop(L, 1);
        
        lua_pushvalue(L, 1);
        if (lua_isfunction(L, -1)) {
            lua_pushvalue(L, -2);
            int err = lua_pcall(L, 1, 0, 0);
            if (err != 0) {
                luaL_error(L,"notification createListener error");
            }
        } else {
            lua_pop(L, 1);
        }
        lua_pop(L, 2);
    } else {
        listenerWrapper ** listener = new (listenerWrapper *);
        pushStrongUserdataTable(L);
        lua_pushlightuserdata(L, listener);
        lua_pushvalue(L, 1);
        lua_rawset(L, -3);
        lua_pop(L, 2);
        
        
        BusinessThread::PostTask(BusinessThread::UI, FROM_HERE, base::BindLambda([=](){
            scoped_refptr<LuaNotificationListener> l = make_scoped_refptr(new LuaNotificationListener());
            l->threadId = now_thread_identifier;
            *listener = new listenerWrapper();
            (*listener)->listener = l;
            BusinessThread::PostTask(now_thread_identifier, FROM_HERE, base::BindLambda([=](){
                lua_State * fromState = BusinessThread::GetCurrentThreadLuaState();
                size_t nbytes = sizeof(listenerWrapper **);
                listenerWrapper ** instanceUserdata = (listenerWrapper **)lua_newuserdata(fromState, nbytes);
                *instanceUserdata = *listener;
                luaL_getmetatable(fromState, LUA_NOTIFICATION_METATABLE_NAME);
                lua_setmetatable(fromState, -2);
                
                lua_newtable(fromState);
                lua_pushcfunction(fromState, AddObserver);
                lua_setfield(fromState, -2, "AddObserver");
                lua_pushcfunction(fromState, RemoveObserver);
                lua_setfield(fromState, -2, "RemoveObserver");
                lua_setfenv(fromState, -2);
                
                pushWeakUserdataTable(fromState);
                lua_pushlightuserdata(fromState, l.get());
                lua_pushvalue(fromState, -3);
                lua_rawset(fromState, -3);
                lua_pop(fromState, 1);
                
                pushUserdataInStrongTable(fromState, listener);
                if (lua_isfunction(fromState, -1)) {
                    lua_pushvalue(fromState, -2);
                    int err = lua_pcall(fromState, 1, 0, 0);
                    if (err != 0) {
                        luaL_error(fromState,"notification createListener error");
                    }
                } else {
                    lua_pop(fromState, 1);
                }
                lua_pop(fromState, 1);
                
                pushStrongUserdataTable(fromState);
                lua_pushlightuserdata(fromState, listener);
                lua_pushnil(fromState);
                lua_settable(fromState, -3);
                lua_pop(fromState, 1);
                delete listener;
            }));
        }));
    }
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int __newindex(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    // Add value to the userdata's environment table.
    lua_getfenv(L, 1);
    lua_insert(L, 2);
    lua_rawset(L, 2);
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int __gc(lua_State *L)
{
    listenerWrapper **instanceUserdata = (listenerWrapper **)luaL_checkudata(L, 1, LUA_NOTIFICATION_METATABLE_NAME);
    listenerWrapper *pointer = *instanceUserdata;
    BusinessThreadID now_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
    if (now_thread_identifier == BusinessThread::UI) {
        delete pointer;
    } else {
        BusinessThread::PostTask(BusinessThread::UI, FROM_HERE, base::BindLambda([=](){
            delete pointer;
        }));
    }
    return 0;
}

static int __index(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    lua_getfenv(L, -2);
    lua_pushvalue(L, -2);
    lua_rawget(L, 3);
    lua_insert(L, 1);
    lua_settop(L, 1);
    END_STACK_MODIFY(L, 1)
    return 1;
}

static const struct luaL_Reg metaFunctions[] = {
    {"__newindex",__newindex},
    {"__index",__index},
    {"__gc", __gc},
    {NULL, NULL}
};

static const struct luaL_Reg functions[] = {
    {"createListener",createListener},
    {"postNotification",postNotification},
    {NULL, NULL}
};

extern int luaopen_notification(lua_State* L){
    BEGIN_STACK_MODIFY(L);
    luaL_newmetatable(L, LUA_NOTIFICATION_METATABLE_NAME);
    luaL_register(L, NULL, metaFunctions);
    luaL_register(L, LUA_NOTIFICATION_METATABLE_NAME, functions);
    END_STACK_MODIFY(L, 0)
    return 0;
}
