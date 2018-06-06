#pragma once
extern "C" {
#include "lua.h"
}
#define LUA_TIMER_METATABLE_NAME "lua.timer"
extern int luaopen_timer(lua_State* L);

