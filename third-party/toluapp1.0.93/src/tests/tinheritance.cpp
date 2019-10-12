extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

int main ()
{
	int errcode = 0;
	int  tolua_tinheritance_open (lua_State*);

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	tolua_tinheritance_open(L);

	if (luaL_dofile(L,"tinheritance.lua") != 0) {
		fprintf(stderr, "%s", lua_tostring(L,-1));
		errcode = 1;
	}

	lua_close(L);

	return errcode;
}

