extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "tools/lua_helpers.h"
#include "lua_file.h"
#include "base/file_util.h"

static int ComputeDirectorySize(lua_State *L);
static int DeleteFile(lua_State *L);
static int Move(lua_State *L);
static int ReplaceFile(lua_State *L);
static int CopyFile(lua_State *L);
static int CopyDirectory(lua_State *L);
static int PathExists(lua_State *L);
static int DirectoryExists(lua_State *L);
static int ContentsEqual(lua_State *L);
static int TextContentsEqual(lua_State *L);
static int ReadFile(lua_State *L);
static int IsDirectoryEmpty(lua_State *L);
static int CreateDirectory(lua_State *L);
static int GetFileSize(lua_State *L);

static int ComputeDirectorySize(lua_State *L)
{
    const char * path = luaL_checkstring(L, 1);
    base::FilePath fpath = base::FilePath(path);
    int size = (int)base::ComputeDirectorySize(fpath);
    lua_pop(L, 1);
    lua_pushinteger(L, size);
    return 1;
}

static int DeleteFile(lua_State *L)
{
    const char * path = luaL_checkstring(L, 1);
    base::FilePath fpath = base::FilePath(path);
    bool b = base::DeleteFile(fpath, true);
    lua_pop(L, 1);
    lua_pushboolean(L, b);
    return 1;
}

static int Move(lua_State *L)
{
    const char * fromPath = luaL_checkstring(L, 1);
    base::FilePath fromFpath = base::FilePath(fromPath);
    const char * toPath = luaL_checkstring(L, 2);
    base::FilePath toFpath = base::FilePath(toPath);
    bool b = base::Move(fromFpath, toFpath);
    lua_pop(L, 2);
    lua_pushboolean(L, b);
    return 1;
}

static int ReplaceFile(lua_State *L)
{
    const char * fromPath = luaL_checkstring(L, 1);
    base::FilePath fromFpath = base::FilePath(fromPath);
    const char * toPath = luaL_checkstring(L, 2);
    base::FilePath toFpath = base::FilePath(toPath);
    bool b = base::ReplaceFile(fromFpath, toFpath, nullptr);
    lua_pop(L, 2);
    lua_pushboolean(L, b);
    return 1;
}

static int CopyFile(lua_State *L)
{
    const char * fromPath = luaL_checkstring(L, 1);
    base::FilePath fromFpath = base::FilePath(fromPath);
    const char * toPath = luaL_checkstring(L, 2);
    base::FilePath toFpath = base::FilePath(toPath);
    bool b = base::CopyFile(fromFpath, toFpath);
    lua_pop(L, 2);
    lua_pushboolean(L, b);
    return 1;
}

static int CopyDirectory(lua_State *L)
{
    const char * fromPath = luaL_checkstring(L, 1);
    base::FilePath fromFpath = base::FilePath(fromPath);
    const char * toPath = luaL_checkstring(L, 2);
    base::FilePath toFpath = base::FilePath(toPath);
    bool b = base::CopyDirectory(fromFpath, toFpath, true);
    lua_pop(L, 2);
    lua_pushboolean(L, b);
    return 1;
}

static int PathExists(lua_State *L)
{
    const char * path = luaL_checkstring(L, 1);
    base::FilePath fpath = base::FilePath(path);
    bool b = base::PathExists(fpath);
    lua_pop(L, 1);
    lua_pushboolean(L, b);
    return 1;
}

static int DirectoryExists(lua_State *L)
{
    const char * path = luaL_checkstring(L, 1);
    base::FilePath fpath = base::FilePath(path);
    bool b = base::DirectoryExists(fpath);
    lua_pop(L, 1);
    lua_pushboolean(L, b);
    return 1;
}

static int ContentsEqual(lua_State *L)
{
    const char * fromPath = luaL_checkstring(L, 1);
    base::FilePath fromFpath = base::FilePath(fromPath);
    const char * toPath = luaL_checkstring(L, 2);
    base::FilePath toFpath = base::FilePath(toPath);
    bool b = base::ContentsEqual(fromFpath, toFpath);
    lua_pop(L, 2);
    lua_pushboolean(L, b);
    return 1;
}

static int TextContentsEqual(lua_State *L)
{
    const char * fromPath = luaL_checkstring(L, 1);
    base::FilePath fromFpath = base::FilePath(fromPath);
    const char * toPath = luaL_checkstring(L, 2);
    base::FilePath toFpath = base::FilePath(toPath);
    bool b = base::TextContentsEqual(fromFpath, toFpath);
    lua_pop(L, 2);
    lua_pushboolean(L, b);
    return 1;
}

static int ReadFile(lua_State *L)
{
    const char * path = luaL_checkstring(L, 1);
    base::FilePath fpath = base::FilePath(path);
    std::string content;
    lua_pop(L, 1);
    bool b = base::ReadFileToString(fpath, &content);
    if (b) {
        lua_pushnil(L);
    } else {
        lua_pushlstring(L, content.c_str(), content.length());
    }
    return 1;
}

static int IsDirectoryEmpty(lua_State *L)
{
    const char * path = luaL_checkstring(L, 1);
    base::FilePath fpath = base::FilePath(path);
    bool b = base::IsDirectoryEmpty(fpath);
    lua_pop(L, 1);
    lua_pushboolean(L, b);
    return 1;
}

static int CreateDirectory(lua_State *L)
{
    const char * path = luaL_checkstring(L, 1);
    base::FilePath fpath = base::FilePath(path);
    bool b = base::CreateDirectory(fpath);
    lua_pop(L, 1);
    lua_pushboolean(L, b);
    return 1;
}

static int GetFileSize(lua_State *L)
{
    const char * path = luaL_checkstring(L, 1);
    base::FilePath fpath = base::FilePath(path);
    int64 size;
    bool b = base::GetFileSize(fpath, &size);
    lua_pop(L, 1);
    if (b) {
        lua_pushinteger(L, (lua_Integer)size);
    } else {
        lua_pushinteger(L, 0);
    }
    return 1;
}

static const struct luaL_Reg metaFunctions[] = {
    {NULL, NULL}
};

static const struct luaL_Reg functions[] = {
    {"ComputeDirectorySize", ComputeDirectorySize},
    {"DeleteFile", DeleteFile},
    {"Move", Move},
    {"ReplaceFile", ReplaceFile},
    {"CopyFile", CopyFile},
    {"CopyDirectory", CopyDirectory},
    {"PathExists", PathExists},
    {"DirectoryExists", DirectoryExists},
    {"ContentsEqual", ContentsEqual},
    {"TextContentsEqual", TextContentsEqual},
    {"ReadFile", ReadFile},
    {"IsDirectoryEmpty", IsDirectoryEmpty},
    {"CreateDirectory", CreateDirectory},
    {"GetFileSize", GetFileSize},
    {NULL, NULL}
};

extern int luaopen_file(lua_State* L){
    BEGIN_STACK_MODIFY(L);
    luaL_newmetatable(L, LUA_FILE_METATABLE_NAME);
    luaL_register(L, NULL, metaFunctions);
    luaL_register(L, LUA_FILE_METATABLE_NAME, functions);
    END_STACK_MODIFY(L, 0)
    return 0;
}
