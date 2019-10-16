extern "C" {
#include "mobdebug.h"
}
#include "lua-tools/lua_helpers.h"

class DebugExtension : LuakitExtension
{
public:
    /* ctor */ DebugExtension()
    {
        license = MIT;
        extensionName = "Debug";
        needChromium = true;
    }
    virtual void LuaOpen(lua_State* L)
    {
        luaopen_mobdebug_scripts(L);
    }
} TheDebugExtension;


