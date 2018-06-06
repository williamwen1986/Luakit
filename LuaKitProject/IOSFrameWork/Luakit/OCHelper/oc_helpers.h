#import <Foundation/Foundation.h>
#ifdef __cplusplus
extern "C" {
#endif
#import "lua.h"
#ifdef __cplusplus
}
#endif

void oc_fromObjc(lua_State *L, id object);

id oc_copyToObjc(lua_State *L, int stackIndex);

void post_notification(int type, id data);
