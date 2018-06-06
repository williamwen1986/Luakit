#pragma once
extern "C" {
#include "lua.h"
}
#define LUA_ASYNC_SOCKET_METATABLE_NAME "lua.asyncSocket"

extern int luaopen_async_socket(lua_State* L);

