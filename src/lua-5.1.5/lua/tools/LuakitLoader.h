#ifndef __LUAKIT_LOADER_H__
#define __LUAKIT_LOADER_H__



extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luakit_loader(lua_State *L);
}

#endif // __LUAKIT_LOADER_H__
