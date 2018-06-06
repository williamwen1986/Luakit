#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#ifdef __cplusplus
}
#endif
#define BEGIN_STACK_MODIFY(L)    int __startStackIndex = lua_gettop((L));
#define END_STACK_MODIFY(L, i) while(lua_gettop((L)) > (__startStackIndex + (i))) lua_remove((L), __startStackIndex + 1);

extern void pushWeakUserdataTable(lua_State *L);
extern void pushStrongUserdataTable(lua_State *L);
extern void pushUserdataInStrongTable(lua_State *L, void * object);
extern void pushUserdataInWeakTable(lua_State *L, void * object);
extern int  luaInit(lua_State* L);
extern void luaSetPackagePath(const char * path);
extern void doString(lua_State* L,const char * s);
extern void luaError (lua_State *L, const char *error);
extern void doString(lua_State* L,const char * s);
extern void setXXTEAKeyAndSign(const char *key, int keyLen, const char *sign, int signLen);
extern int luaLoadBuffer(lua_State *L, const char *chunk, int chunkSize, const char *chunkName);
