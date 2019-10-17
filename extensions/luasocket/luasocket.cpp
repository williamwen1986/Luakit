extern "C" 
{
	#include "luasocket.h"
}

#include "lua-tools/lua_helpers.h"



class SocketExtension : LuakitExtension
{
public:
    /* ctor */ SocketExtension()
    {
        license = MIT;
        extensionName = "Socket";
        needChromium = false;
    }
    virtual void LuaOpen(lua_State* L)
    {
        luaopen_socket_core(L);
    }
} TheSocketExtension;


