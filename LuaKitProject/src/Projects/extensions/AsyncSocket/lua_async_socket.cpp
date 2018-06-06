extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "lua_async_socket.h"
#include "tools/lua_helpers.h"
#include "network/async_socket.h"
#include "network/net/io_buffer.h"
#include "common/base_lambda_support.h"
static int create(lua_State *L);
static int __index(lua_State *L);
static int __newindex(lua_State *L);
static int __gc(lua_State *L);
static int connect(lua_State *L);
static int read(lua_State *L);
static int write(lua_State *L);
static int disconnect(lua_State *L);
static int isConnected(lua_State *L);

static int create(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    const char * host = luaL_checkstring(L, 1);
    int port = luaL_checkint(L, 2);
    lua_pop(L, 2);
    net::AsyncSocket * socket = new net::AsyncSocket(host,port);
    size_t nbytes = sizeof(net::AsyncSocket **);
    net::AsyncSocket ** instanceUserdata = (net::AsyncSocket **)lua_newuserdata(L, nbytes);
    *instanceUserdata = socket;
    luaL_getmetatable(L, LUA_ASYNC_SOCKET_METATABLE_NAME);
    lua_setmetatable(L, -2);
    lua_newtable(L);
    
    lua_pushcfunction(L, connect);
    lua_setfield(L, -2, "connect");
    lua_pushcfunction(L, read);
    lua_setfield(L, -2, "read");
    lua_pushcfunction(L, write);
    lua_setfield(L, -2, "write");
    lua_pushcfunction(L, disconnect);
    lua_setfield(L, -2, "disconnect");
    lua_pushcfunction(L, isConnected);
    lua_setfield(L, -2, "isConnected");
    
    lua_setfenv(L, -2);
    
    pushWeakUserdataTable(L);
    lua_pushlightuserdata(L, *instanceUserdata);
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    
    END_STACK_MODIFY(L, 1)
    return 1;
}

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

static int __gc(lua_State *L)
{
    net::AsyncSocket **instanceUserdata = (net::AsyncSocket **)luaL_checkudata(L, 1, LUA_ASYNC_SOCKET_METATABLE_NAME);
    delete *instanceUserdata;
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
    END_STACK_MODIFY(L, 1);
    return 1;
}

static int connect(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    net::AsyncSocket **instanceUserdata = (net::AsyncSocket **)luaL_checkudata(L, 1, LUA_ASYNC_SOCKET_METATABLE_NAME);
    (*instanceUserdata)->connect(base::BindLambda([=](int rv){
        pushUserdataInWeakTable(L,*instanceUserdata);
        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "connectCallback");
            if (lua_isfunction(L, -1)) {
                lua_pushnumber(L, rv);
                int err = lua_pcall(L, 1, 0, 0);
                if (err != 0) {
                    luaL_error(L,"async_socket connectCallback error");
                }
            } else {
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);
    }));
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int read(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    net::AsyncSocket **instanceUserdata = (net::AsyncSocket **)luaL_checkudata(L, 1, LUA_ASYNC_SOCKET_METATABLE_NAME);
    auto buffer = make_scoped_refptr(new net::IOBufferWithSize(8192));
    (*instanceUserdata)->read(buffer, buffer->size(),base::BindLambda([=](int rv){
        if (rv > 0) {
            pushUserdataInWeakTable(L,*instanceUserdata);
            if (!lua_isnil(L, -1)) {
                lua_getfield(L, -1, "readCallback");
                if (lua_isfunction(L, -1)) {
                    lua_pushlstring(L, (char *)(buffer->data()), rv);
                    int err = lua_pcall(L, 1, 0, 0);
                    if (err != 0) {
                        luaL_error(L,"async_socket readCallback error");
                    }
                } else {
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);
        }
    }));
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int write(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    net::AsyncSocket **instanceUserdata = (net::AsyncSocket **)luaL_checkudata(L, 1, LUA_ASYNC_SOCKET_METATABLE_NAME);
    std::string data = luaL_checkstring(L, 2);
    auto buffer = make_scoped_refptr(new net::StringIOBuffer(data));
    (*instanceUserdata)->write(buffer, data.length(),base::BindLambda([=](int rv){
        pushUserdataInWeakTable(L,*instanceUserdata);
        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "writeCallback");
            if (lua_isfunction(L, -1)) {
                lua_pushnumber(L, rv);
                int err = lua_pcall(L, 1, 0, 0);
                if (err != 0) {
                    luaL_error(L,"async_socket writeCallback error");
                }
            } else {
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);
    }));
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int disconnect(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    net::AsyncSocket **instanceUserdata = (net::AsyncSocket **)luaL_checkudata(L, 1, LUA_ASYNC_SOCKET_METATABLE_NAME);
    (*instanceUserdata)->disconnect();
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int isConnected(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    net::AsyncSocket **instanceUserdata = (net::AsyncSocket **)luaL_checkudata(L, 1, LUA_ASYNC_SOCKET_METATABLE_NAME);
    lua_settop(L, 0);
    lua_pushboolean(L, (*instanceUserdata)->isConnected());
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
    {"create", create},
    {NULL, NULL}
};

extern int luaopen_async_socket(lua_State* L){
    BEGIN_STACK_MODIFY(L);
    luaL_newmetatable(L, LUA_ASYNC_SOCKET_METATABLE_NAME);
    luaL_register(L, NULL, metaFunctions);
    luaL_register(L, LUA_ASYNC_SOCKET_METATABLE_NAME, functions);
    END_STACK_MODIFY(L, 0)
    return 0;
}
