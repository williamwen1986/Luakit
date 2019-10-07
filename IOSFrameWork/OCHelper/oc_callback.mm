#import "oc_helpers.h"
#import <objc/runtime.h>
#ifdef __cplusplus
extern "C" {
#endif
#import "lauxlib.h"
#import "lapi.h"
#ifdef __cplusplus
}
#endif
#import "lua_helpers.h"
#import "oc_callback.h"

typedef struct _callback_instance_userdata {
    void (^callback) (id o);
} callback_instance_userdata;

#define LUA_OC_CALLBACK_METATABLE_NAME "lua.occallback"

static int __gc(lua_State *L);
static int __call(lua_State *L);

static int __gc(lua_State *L)
{
    callback_instance_userdata *instanceUserdata = (callback_instance_userdata *)luaL_checkudata(L, 1, LUA_OC_CALLBACK_METATABLE_NAME);
    [instanceUserdata->callback release];
    return 0;
}

static int __call(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    callback_instance_userdata *instanceUserdata = (callback_instance_userdata *)luaL_checkudata(L, 1, LUA_OC_CALLBACK_METATABLE_NAME);
    id params = oc_copyToObjc(L,2);
    instanceUserdata->callback(params);
    return 0;
}

static const struct luaL_Reg metaFunctions[] = {
    {"__gc", __gc},
    {"__call", __call},
    {NULL, NULL}
};

static const struct luaL_Reg functions[] = {
    {NULL, NULL}
};

extern int luaopen_oc_callback(lua_State* L)
{
    BEGIN_STACK_MODIFY(L);
    luaL_newmetatable(L, LUA_OC_CALLBACK_METATABLE_NAME);
    luaL_register(L, NULL, metaFunctions);
    luaL_register(L, LUA_OC_CALLBACK_METATABLE_NAME, functions);
    END_STACK_MODIFY(L, 0)
    return 0;
}

extern void push_oc_block(lua_State* L, void (^callback) (id o))
{
    luaL_getmetatable(L, LUA_OC_CALLBACK_METATABLE_NAME);
    if (lua_isnil(L, -1)) {
        luaopen_oc_callback(L);
    }
    lua_pop(L, 1);
    size_t nbytes = sizeof(callback_instance_userdata);
    callback_instance_userdata *instanceUserdata = (callback_instance_userdata *)lua_newuserdata(L, nbytes);
    instanceUserdata->callback = callback;
    [callback retain];
    luaL_getmetatable(L, LUA_OC_CALLBACK_METATABLE_NAME);
    lua_setmetatable(L, -2);
}
