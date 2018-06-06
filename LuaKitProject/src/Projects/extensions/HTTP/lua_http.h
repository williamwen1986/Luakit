#pragma once
extern "C" {
#include "lua.h"
}
#define LUA_HTTP_METATABLE_NAME "lua.http"

extern int luaopen_http(lua_State* L);

