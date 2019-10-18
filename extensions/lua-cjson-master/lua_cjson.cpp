extern "C" {
#include "lua_cjson.h"
}
#include "lua-tools/lua_helpers.h"
class CjsonExtension : LuakitExtension
{
public:
    /* ctor */ CjsonExtension() 
    {
        license = MIT;
        extensionName = "Cjson";
        needChromium = true;
    }
    virtual void LuaOpen(lua_State* L)
    {
        //if (!isOpen)
        {
            isOpen = true;
            luaopen_cjson(L);
            luaopen_cjson_safe(L);
        }
	}
} TheCjsonExtension;
