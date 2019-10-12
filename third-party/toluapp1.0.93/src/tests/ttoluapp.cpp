extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "ttoluapp.h"

//Test::Tst_A* Test::Tst_A::last;
//Test::Tst_B* Test::Tst_B::last;
//Test::Tst_C* Test::Tst_C::last;

//extern "C" {
	int  tolua_ttoluapp_open (lua_State*);
//}

int main ()
{
	int errcode = 0;
	Test::Tst_B* b = new Test::Tst_B;         // instance used in Lua code

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	tolua_ttoluapp_open(L);

	if (luaL_dofile(L,"ttoluapp.lua") != 0) {
		fprintf(stderr, "%s", lua_tostring(L,-1));
		errcode = 1;
	}

	lua_close(L);

	delete b;
	return errcode;
}

