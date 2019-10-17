#include "lua-tools/lua_helpers.h"

class NetworkExtension : LuakitExtension
{
public:
    /* ctor */ NetworkExtension() 
    {
        license = MIT;
        extensionName = "Network";
        needChromium = true;
    }
    virtual void LuaOpen(lua_State* L)
    {
    }
} TheNetworkExtension;

