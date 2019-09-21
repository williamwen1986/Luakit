#pragma once
extern "C" {
#include "lua.h"
}
#define LUA_LANGUAGE_METATABLE_NAME "lua_language"
extern int luaopen_language(lua_State* L);

