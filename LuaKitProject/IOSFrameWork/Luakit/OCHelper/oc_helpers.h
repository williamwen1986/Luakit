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

id call_lua_function(NSString * moduleName , NSString * MethodName);

id call_lua_function(NSString * moduleName , NSString * MethodName , id params);

id call_lua_function(NSString * moduleName , NSString * MethodName , id params1, id params2);

id call_lua_function(NSString * moduleName , NSString * MethodName , id params1, id params2, id params3);

id call_lua_function(NSString * moduleName , NSString * MethodName , id params1, id params2, id params3, id params4);

id call_lua_function(NSString * moduleName , NSString * MethodName , id params1, id params2, id params3, id params4, id params5);
