#ifdef __cplusplus
extern "C" {
#endif
#include "lauxlib.h"
#include "lualib.h"
#include "lsqlite3.h"
#include "lua_cjson.h"
#ifdef __cplusplus
}
#endif
#include "lua_helpers.h"
#include "lua_http.h"
#include "lua_async_socket.h"
#include "lua_timer.h"
#include "lua_thread.h"
#include "base/path_service.h"
#include "base/files/file_path.h"
#include "lua_file.h"
#include "lua_notify.h"
#include "LuakitLoader.h"
#include "xxtea.h"

static bool  _xxteaEnabled = false;
static char* _xxteaKey = NULL;
static int   _xxteaKeyLen = 0;
static char* _xxteaSign = NULL;
static int   _xxteaSignLen = 0;

#if defined(OS_IOS)
static const int PATH_SERVICE_KEY = base::DIR_DOCUMENTS;
#elif defined(OS_ANDROID)
static const int PATH_SERVICE_KEY = base::DIR_ANDROID_APP_DATA;
#endif

static std::string packagePath = "";

extern void pushWeakUserdataTable(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    static const char* userdataTableName = "__weak_userdata";
    luaL_getmetatable(L, LUA_CALLBACK_METATABLE_NAME);
    lua_getfield(L, -1, userdataTableName);
    if (lua_isnil(L, -1)) { // Create new userdata table, add it to metatable
        lua_pop(L, 1); // Remove nil
        lua_pushstring(L, userdataTableName); // Table name
        lua_newtable(L);
        lua_rawset(L, -3); // Add userdataTableName table to LUA_CALLBACK_METATABLE_NAME
        lua_getfield(L, -1, userdataTableName);
        lua_pushvalue(L, -1);
        lua_setmetatable(L, -2); // userdataTable is it's own metatable
        lua_pushstring(L, "v");
        lua_setfield(L, -2, "__mode");  // Make weak table
    }
    END_STACK_MODIFY(L, 1)
}

extern void pushStrongUserdataTable(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    static const char* userdataTableName = "__strong_userdata";
    luaL_getmetatable(L, LUA_CALLBACK_METATABLE_NAME);
    lua_getfield(L, -1, userdataTableName);
    if (lua_isnil(L, -1)) { // Create new userdata table, add it to metatable
        lua_pop(L, 1); // Remove nil
        lua_pushstring(L, userdataTableName); // Table name
        lua_newtable(L);
        lua_rawset(L, -3); // Add userdataTableName table to LUA_CALLBACK_METATABLE_NAME
        lua_getfield(L, -1, userdataTableName);
    }
    END_STACK_MODIFY(L, 1)
}

extern void pushUserdataInStrongTable(lua_State *L, void * object) {
    BEGIN_STACK_MODIFY(L)
    pushStrongUserdataTable(L);
    lua_pushlightuserdata(L, object);
    lua_rawget(L, -2);
    lua_remove(L, -2);
    END_STACK_MODIFY(L, 1)
}

int lua_panic(lua_State *L) {
    LOG(ERROR)<<"Lua panicked and quit:\n"<<luaL_checkstring(L, -1);
    return 0;
}

int lua_err(const char *result) {
    LOG(ERROR)<<"lua_err \n"<<result;
    return 0;
}

extern void  luaError (lua_State *L, const char *error)
{
    luaL_error(L, error);
}

extern void doString(lua_State* L,const char * s)
{
    luaL_dostring(L, s);
}

extern void luaSetPackagePath(const char * path)
{
    packagePath = path;
}

#if defined(OS_ANDROID)
static int androidPrint(lua_State *L) {
    LOG(WARNING)<<"androidPrint:"<<luaL_checkstring(L, 1);
    return 0;
}
#endif

static void cleanupXXTEAKeyAndSign()
{
    if (_xxteaKey)
    {
        free(_xxteaKey);
        _xxteaKey = nullptr;
        _xxteaKeyLen = 0;
    }
    if (_xxteaSign)
    {
        free(_xxteaSign);
        _xxteaSign = nullptr;
        _xxteaSignLen = 0;
    }
}

extern void setXXTEAKeyAndSign(const char *key, int keyLen, const char *sign, int signLen)
{
    cleanupXXTEAKeyAndSign();
    
    if (key && keyLen && sign && signLen)
    {
        _xxteaKey = (char*)malloc(keyLen);
        memcpy(_xxteaKey, key, keyLen);
        _xxteaKeyLen = keyLen;
        
        _xxteaSign = (char*)malloc(signLen);
        memcpy(_xxteaSign, sign, signLen);
        _xxteaSignLen = signLen;
        
        _xxteaEnabled = true;
    }
    else
    {
        _xxteaEnabled = false;
    }
}

static void skipBOM(const char*& chunk, int& chunkSize)
{
    // UTF-8 BOM? skip
    if (static_cast<unsigned char>(chunk[0]) == 0xEF &&
        static_cast<unsigned char>(chunk[1]) == 0xBB &&
        static_cast<unsigned char>(chunk[2]) == 0xBF)
    {
        chunk += 3;
        chunkSize -= 3;
    }
}

extern int luaLoadBuffer(lua_State *L, const char *chunk, int chunkSize, const char *chunkName)
{
    int r = 0;
    
    if (_xxteaEnabled && strncmp(chunk, _xxteaSign, _xxteaSignLen) == 0)
    {
        // decrypt XXTEA
        xxtea_long len = 0;
        unsigned char* result = xxtea_decrypt((unsigned char*)chunk + _xxteaSignLen,
                                              (xxtea_long)chunkSize - _xxteaSignLen,
                                              (unsigned char*)_xxteaKey,
                                              (xxtea_long)_xxteaKeyLen,
                                              &len);
        unsigned char* content = result;
        xxtea_long contentSize = len;
        skipBOM((const char*&)content, (int&)contentSize);
        r = luaL_loadbuffer(L, (char*)content, contentSize, chunkName);
        free(result);
    }
    else
    {
        skipBOM(chunk, chunkSize);
        r = luaL_loadbuffer(L, chunk, chunkSize, chunkName);
    }
    
    if (r)
    {
        switch (r)
        {
            case LUA_ERRSYNTAX:
                LOG(ERROR)<<"[LUA ERROR] load , error: syntax error during pre-compilation."<< chunkName;
                break;
                
            case LUA_ERRMEM:
                LOG(ERROR)<<"[LUA ERROR] load , error: memory allocation error."<< chunkName;
                break;
                
            case LUA_ERRFILE:
                LOG(ERROR)<<"[LUA ERROR] load , error: cannot open/read file."<< chunkName;
                break;
                
            default:
                LOG(ERROR)<<"[LUA ERROR] load , error: unknown."<< chunkName;
        }
    }
    return r;
}

static void addLuaLoader(lua_State* L, lua_CFunction func)
{
    if (!func) return;
    
    // stack content after the invoking of the function
    // get loader table
    lua_getglobal(L, "package");                                  /* L: package */
    lua_getfield(L, -1, "loaders");                               /* L: package, loaders */
    
    // insert loader into index 2
    lua_pushcfunction(L, func);                                   /* L: package, loaders, func */
    for (int i = (int)(lua_objlen(L, -2) + 1); i > 2; --i)
    {
        lua_rawgeti(L, -2, i - 1);                                /* L: package, loaders, func, function */
        // we call lua_rawgeti, so the loader table now is at -3
        lua_rawseti(L, -3, i);                                    /* L: package, loaders, func */
    }
    lua_rawseti(L, -2, 2);                                        /* L: package, loaders */
    
    // set loaders into package
    lua_setfield(L, -2, "loaders");                               /* L: package */
    
    lua_pop(L, 1);
}

extern int luaInit(lua_State* L)
{
    lua_atpanic(L, &lua_panic);
    lua_aterr(L, &lua_err);
    luaL_openlibs(L);
    luaopen_file(L);
    luaopen_lsqlite3(L);
    luaopen_http(L);
    luaopen_callback(L);
    luaopen_thread(L);
    luaopen_timer(L);
    luaopen_cjson(L);
    luaopen_cjson_safe(L);
    luaopen_async_socket(L);
    luaopen_notification(L);
    addLuaLoader(L,luakit_loader);
    base::FilePath documentDir;
    PathService::Get(PATH_SERVICE_KEY, &documentDir);
    std::string path = documentDir.value();
    LOG(WARNING)<<"documentDir:"<<path;
    lua_pushstring(L, path.c_str());
    lua_setglobal(L, "BASE_DOCUMENT_PATH");
    
#if defined(OS_ANDROID)
    lua_pushcfunction(L, androidPrint);
    lua_setglobal(L, "print");
#endif
    
    if (packagePath.length() > 0) {
        std::string lua = "package.path = '" +packagePath+"/?.lua'";
        luaL_dostring(L, lua.c_str());
    }
    
#if defined(OS_ANDROID)
    std::string lua = "package.path = '" +path+"/lua/?.lua'";
    luaL_dostring(L, lua.c_str());
#endif
    return 0;
}

extern void pushUserdataInWeakTable(lua_State *L, void * object){
    BEGIN_STACK_MODIFY(L)
    pushWeakUserdataTable(L);
    lua_pushlightuserdata(L, object);
    lua_rawget(L, -2);
    lua_remove(L, -2);
    END_STACK_MODIFY(L, 1)
}

