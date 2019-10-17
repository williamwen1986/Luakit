extern "C" 
{
#include "lsqlite3.h"
}

#include "lua-tools/lua_helpers.h"

class SQLiteExtension : LuakitExtension
{
public:
    /* ctor */ SQLiteExtension()
    {
        license = MIT;
        extensionName = "Sqlite";
        needChromium = false;
    }
    virtual void LuaOpen(lua_State* L)
    {
        luaopen_lsqlite3(L);
    }
} TheSQLiteExtension;


