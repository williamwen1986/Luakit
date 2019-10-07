extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "tools/lua_helpers.h"
#include "lua_http.h"
#include "network/async_cgi_task_dispatcher.h"
#include "common/base_lambda_support.h"
#include "lua_http_task.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread_restrictions.h"

static scoped_refptr<network::HttpCgiTaskDispatcher> dispatcher = NULL;
static int request(lua_State *L);

static int __newindex(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    // Add value to the userdata's environment table.
    lua_getfenv(L, 1);
    lua_insert(L, 2);
    lua_rawset(L, 2);
    END_STACK_MODIFY(L, 0);
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
    {NULL, NULL}
};

static const struct luaL_Reg functions[] = {
    {"request", request},
    {NULL, NULL}
};

static HTTP_HEADERS getHeaders(lua_State *L) {
    HTTP_HEADERS headers;
    if (lua_isnoneornil(L, -1)) return headers;
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        std::string k = luaL_checkstring(L, -2);
        std::string v = luaL_checkstring(L, -1);
        headers.push_back(std::pair<std::string, std::string>(k,v));
        lua_pop(L, 1); // Pop off the value
    }
    return headers;
}

static bool pushCallback(lua_State *L, const char *callbackName, int tableIndex) {
    lua_getfield(L, tableIndex, callbackName);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    else {
        return true;
    }
}

static int request(lua_State *L) {
    BEGIN_STACK_MODIFY(L)
    std::string taskName = "";
    lua_getfield(L, 1, "taskName");
    if(!lua_isnil(L, -1)){
        taskName = luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);
    
    std::string url = "";
    lua_getfield(L, 1, "url");
    if(!lua_isnil(L, -1)){
        url = luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);
    
    bool isPost = false;
    lua_getfield(L, 1, "isPost");
    if(!lua_isnil(L, -1)){
        isPost = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);
    
    std::string uploadContent = "";
    lua_getfield(L, 1, "uploadContent");
    if(!lua_isnil(L, -1)){
        uploadContent = luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);
    
    std::string uploadPath = "";
    lua_getfield(L, 1, "uploadPath");
    if(!lua_isnil(L, -1)){
        uploadPath = luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);
    
    std::string downloadPath = "";
    lua_getfield(L, 1, "downloadPath");
    if(!lua_isnil(L, -1)){
        downloadPath = luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);
    
    HTTP_HEADERS headers;
    lua_getfield(L, 1, "headers");
    if(!lua_isnil(L, -1)){
        headers = getHeaders(L);
    }
    lua_pop(L, 1);
    
    uint32_t socketWatcherTimeout = 30000;
    lua_getfield(L, 1, "socketWatcherTimeout");
    if(!lua_isnil(L, -1)){
        socketWatcherTimeout = luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);
    
    network::LuaHttpTask *task = new network::LuaHttpTask(taskName);
    
    BusinessThreadID now_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
    task->threadId = now_thread_identifier;
    task->url = url;
    task->is_post = isPost;
    task->upload_path = uploadPath;
    task->upload_content = uploadContent;
    task->download_path = downloadPath;
    task->socket_watcher_timeout = socketWatcherTimeout;
    task->headers = headers;
    
    size_t nbytes = sizeof(network::LuaHttpTask *);
    network::LuaHttpTask ** instanceUserdata = (network::LuaHttpTask **)lua_newuserdata(L, nbytes);
    *instanceUserdata = task;
    
    luaL_getmetatable(L, LUA_HTTP_METATABLE_NAME);
    lua_setmetatable(L, -2);
    
    // give it a nice clean environment
    lua_newtable(L);
    lua_setfenv(L, -2);
    pushStrongUserdataTable(L);
    lua_pushlightuserdata(L, task);
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    
    if (pushCallback(L, "onResponse", 1)) {
        lua_setfield(L, -2, "onResponse");
    }
    
    if (pushCallback(L, "onProgress", 1)) {
        lua_setfield(L, -2, "onProgress");
    }
    
    lua_pop(L, 1);
    
    BusinessThread::PostTask(BusinessThread::IO, FROM_HERE, base::BindLambda([=]()
    {
        if (dispatcher == NULL) {
            dispatcher = new network::HttpCgiTaskDispatcher(LUA_HTTP_METATABLE_NAME, true);
        }
        dispatcher->ScheduleTask(make_scoped_refptr(task));
    })) ;
    END_STACK_MODIFY(L, 0)
    return 0;
}


extern int luaopen_http(lua_State *L) {
    BEGIN_STACK_MODIFY(L);
    luaL_newmetatable(L, LUA_HTTP_METATABLE_NAME);
    luaL_register(L, NULL, metaFunctions);
    luaL_register(L, LUA_HTTP_METATABLE_NAME, functions);
    lua_pushvalue(L, -2);
    lua_setmetatable(L, -2); // Set the metatable for the module
    END_STACK_MODIFY(L, 0)
    return 0;
}
