#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#ifdef __cplusplus
}
#endif
#include <string>

typedef void (*LuaErrorFun)(const char *);


#define BEGIN_STACK_MODIFY(L)    int __startStackIndex = lua_gettop((L));
#define END_STACK_MODIFY(L, i) while(lua_gettop((L)) > (__startStackIndex + (i))) lua_remove((L), __startStackIndex + 1);

extern void pushWeakUserdataTable(lua_State *L);
extern void pushStrongUserdataTable(lua_State *L);
extern void pushUserdataInStrongTable(lua_State *L, void * object);
extern void pushUserdataInWeakTable(lua_State *L, void * object);
extern int  luaInit(lua_State* L);
extern void setLuaErrorFun(LuaErrorFun func);
extern void doString(lua_State* L,const char * s);
extern void luaError (lua_State *L, const char *error);
extern void doString(lua_State* L,const char * s);
extern void setXXTEAKeyAndSign(const char *key, int keyLen, const char *sign, int signLen);
extern int luaLoadBuffer(lua_State *L, const char *chunk, int chunkSize, const char *chunkName);
// extern void  (lua_getfenv) (lua_State *L, int idx); // Patch [LARPOUX]
// extern int   (lua_setfenv) (lua_State *L, int idx); // Patch [LARPOUX]


extern void luaSetPackagePath(std::string);
extern std::string luaGetPackagePath();

extern void luaSetDataDirectoryPath(std::string path);
extern std::string luaGetDataDirectoryPath();

extern void luaSetDatabaseDirectoryPath(std::string path);
extern std::string luaGetDatabaseDirectoryPath();

extern void luaSetCacheDirectoryPath(std::string path) ;
extern std::string luaGetCacheDirectoryPath(); 

extern void luaSetDownloadDirectoryPath(std::string path);
extern std::string luaGetDownloadDirectoryPath();

extern void luaSetNativeLibraryDirectoryPath(std::string path); 
extern std::string luaGetNativeLibraryDirectoryPath();

extern void luaSetExternalStorageDirectoryPath(std::string path);
extern std::string luaGetExternalStorageDirectoryPath();


