#pragma once
extern "C" {
#include "lua.h"
}
#define LUA_ASYNC_SOCKET_METATABLE_NAME "lua_asyncSocket"

extern int luaopen_async_socket(lua_State* L);

