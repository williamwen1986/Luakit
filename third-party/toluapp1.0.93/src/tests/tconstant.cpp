extern "C"
{
#include "lualib.h"
#include "lauxlib.h"
}

#include "tconstant.h"


int main (void)
{
	int errcode = 0;
	int  tolua_tconstant_open (lua_State*);
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	tolua_tconstant_open(L);

	if (luaL_dofile(L,"tconstant.lua") != 0) {
		fprintf(stderr, "%s", lua_tostring(L,-1));
		errcode = 1;
	}

	lua_close(L);
	return errcode;
}

