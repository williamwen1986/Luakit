#import <Foundation/Foundation.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#ifdef __cplusplus
}
#endif

extern int luaopen_oc_callback(lua_State* L);
extern void push_oc_block(lua_State* L, void (^callback) (id o));
