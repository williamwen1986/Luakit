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
#import "oc_callback.h"
#include "common/notification_service.h"
#include "lua_notify.h"
#include "NotificationProxyObserver.h"
#include "lua_helpers.h"

void pushOneObject(lua_State *L, id object);
id getOneObject(lua_State *L, int stackIndex);
void pushDictObject(lua_State *L, NSDictionary *d)
{
    lua_newtable(L);
    for (int i = 0; i<d.allKeys.count; i++) {
        pushOneObject(L, d.allKeys[i]);
        pushOneObject(L, d.allValues[i]);
        lua_rawset(L, -3);
    }
}

void pushArrayObject(lua_State *L, NSArray* a)
{
    lua_newtable(L);
    for (int i = 0; i<a.count; i++) {
        pushOneObject(L, @(i+1));
        pushOneObject(L, a[i]);
        lua_rawset(L, -3);
    }
}

void pushOneObject(lua_State *L, id object)
{
    if(!object){
        lua_pushnil(L);
    } else if([object isKindOfClass:[NSNumber class]]){
        switch ([object objCType][0]) {
            case _C_CHR:
            case _C_SHT:
            case _C_INT:
            case _C_LNG:
            case _C_LNG_LNG:
                lua_pushinteger(L, [object integerValue]);
                break;
            case _C_USHT:
            case _C_UINT:
            case _C_ULNG:
            case _C_ULNG_LNG:
                lua_pushinteger(L, [object unsignedIntegerValue]);
                break;
            case _C_FLT:
                lua_pushnumber(L, [object floatValue]);
            case _C_DBL:
                lua_pushnumber(L, [object doubleValue]);
                break;
            default:
                lua_pushinteger(L, [object integerValue]);
                break;
        }
    } else if([object isKindOfClass:[NSString class]]) {
        NSString * s = (NSString *)object;
        lua_pushlstring(L, [s cStringUsingEncoding:NSUTF8StringEncoding], s.length);
    } else if([object isKindOfClass:[NSData class]]) {
        NSData * d = (NSData *)object;
        lua_pushlstring(L, (const char *)d.bytes, d.length);
    } else if([object isKindOfClass:[NSArray class]]) {
        pushArrayObject(L, (NSArray *)object);
    } else if([object isKindOfClass:[NSDictionary class]]){
        pushDictObject(L, (NSDictionary *)object);
    } else {
        NSString *className = NSStringFromClass([object class]);
        if ([className containsString:@"Block"]) {
            push_oc_block(L, (void (^)(id o))object);
        }
    }
}

id getTableObject(lua_State *L, int stackIndex){
    BOOL dictionary = NO;
    lua_pushvalue(L, stackIndex);
    lua_pushnil(L);  /* first key */
    while (!dictionary && lua_next(L, -2)) {
        if (lua_type(L, -2) != LUA_TNUMBER) {
            dictionary = YES;
            lua_pop(L, 2); // pop key and value off the stack
        }
        else {
            lua_pop(L, 1);
        }
    }
    id instance = nil;
    if (dictionary) {
        instance = [NSMutableDictionary dictionary];
        
        lua_pushnil(L);  /* first key */
        while (lua_next(L, -2)) {
            id key = getOneObject(L, -2);
            id object = getOneObject(L, -1);
            [instance setObject:object forKey:key];
            lua_pop(L, 1); // Pop off the value
        }
    }
    else {
        instance = [NSMutableArray array];
        
        lua_pushnil(L);  /* first key */
        while (lua_next(L, -2)) {
            int index = lua_tonumber(L, -2) - 1;
            id object = getOneObject(L, -1);
            [instance insertObject:object atIndex:index];
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    return instance;
}

id getOneObject(lua_State *L, int stackIndex)
{
    int type = lua_type(L,stackIndex);
    switch(type) {
        case LUA_TNIL:{
            return nil;
        }
        break;
        case LUA_TBOOLEAN:{
            int ret = lua_toboolean(L, stackIndex);
            return @(ret);
        }
        break;
        case LUA_TNUMBER:{
            lua_Number ret = lua_tonumber(L, stackIndex);
            return @(ret);
        }
        break;
        case LUA_TSTRING:{
            size_t size;
            const char * ret = lua_tolstring(L, stackIndex, &size);
            NSData *d = [NSData dataWithBytes:ret length:size];
            NSString * s = [[NSString alloc] initWithData:d encoding:NSUTF8StringEncoding];
            return s;
        }
        break;
        case LUA_TTABLE:{
            return getTableObject(L, stackIndex);
        }
        break;
        default:
            luaL_error(L, "Unsupport type %s to getOneObject", lua_typename(L, type));
    }
    return nil;
}

void oc_fromObjc(lua_State *L, id object)
{
    pushOneObject(L, object);
}

id oc_copyToObjc(lua_State *L, int stackIndex)
{
    return getOneObject(L, stackIndex);
}

void post_notification(int type, id data)
{
    void (^b)(void) = ^(){
        Notification::sourceType s = Notification::IOS_SYS;
        content::NotificationService::current()->Notify(type,content::Source<Notification::sourceType>(&s),content::Details<void>((__bridge void *)data));
    };
    if([NSThread isMainThread]){
        b();
    } else {
        dispatch_async(dispatch_get_main_queue(), b);
    }
}

void pushLuaModule(NSString * moduleName)
{
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    NSString * lua = [NSString stringWithFormat:@"TEM_OC_OBJECT = require('%@')",moduleName];
    doString(L, [lua cStringUsingEncoding:NSUTF8StringEncoding]);
    lua_getglobal(L, "TEM_OC_OBJECT");
    lua_pushnil(L);
    lua_setglobal(L, "TEM_OC_OBJECT");
    END_STACK_MODIFY(L, 1)
}


id call_lua_function(NSString * moduleName , NSString * MethodName)
{
    if (moduleName.length <= 0 || MethodName.length <= 0) {
        return nil;
    }
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(moduleName);
    lua_pushstring(L, [MethodName cStringUsingEncoding:NSUTF8StringEncoding]);
    lua_rawget(L, -2);
    id ret = nil;
    if (lua_isfunction(L, -1)) {
        int err = lua_pcall(L, 0, 1, 0);
        if (err != 0) {
            luaError(L,"call_lua_function call error");
        } else {
            ret = oc_copyToObjc(L,-1);
        }
    }
    END_STACK_MODIFY(L, 0)
    return ret;
}

id call_lua_function(NSString * moduleName , NSString * MethodName , id params)
{
    if (moduleName.length <= 0 || MethodName.length <= 0) {
        return nil;
    }
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(moduleName);
    lua_pushstring(L, [MethodName cStringUsingEncoding:NSUTF8StringEncoding]);
    lua_rawget(L, -2);
    id ret = nil;
    if (lua_isfunction(L, -1)) {
        oc_fromObjc(L, params);
        int err = lua_pcall(L, 1, 1, 0);
        if (err != 0) {
            luaError(L,"call_lua_function call error");
        } else {
            ret = oc_copyToObjc(L,-1);
        }
    } else {
        luaError(L,"call_lua_function call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    return ret;
}

id call_lua_function(NSString * moduleName , NSString * MethodName , id params1, id params2)
{
    if (moduleName.length <= 0 || MethodName.length <= 0) {
        return nil;
    }
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(moduleName);
    lua_pushstring(L, [MethodName cStringUsingEncoding:NSUTF8StringEncoding]);
    lua_rawget(L, -2);
    id ret = nil;
    if (lua_isfunction(L, -1)) {
        oc_fromObjc(L, params1);
        oc_fromObjc(L, params2);
        int err = lua_pcall(L, 2, 1, 0);
        if (err != 0) {
            luaError(L,"call_lua_function call error");
        } else {
            ret = oc_copyToObjc(L,-1);
        }
    } else {
        luaError(L,"call_lua_function call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    return ret;
}

id call_lua_function(NSString * moduleName , NSString * MethodName , id params1, id params2, id params3)
{
    if (moduleName.length <= 0 || MethodName.length <= 0) {
        return nil;
    }
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(moduleName);
    lua_pushstring(L, [MethodName cStringUsingEncoding:NSUTF8StringEncoding]);
    lua_rawget(L, -2);
    id ret = nil;
    if (lua_isfunction(L, -1)) {
        oc_fromObjc(L, params1);
        oc_fromObjc(L, params2);
        oc_fromObjc(L, params3);
        int err = lua_pcall(L, 3, 1, 0);
        if (err != 0) {
            luaError(L,"call_lua_function call error");
        } else {
            ret = oc_copyToObjc(L,-1);
        }
    } else {
        luaError(L,"call_lua_function call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    return ret;
}

id call_lua_function(NSString * moduleName , NSString * MethodName , id params1, id params2, id params3, id params4) {
    if (moduleName.length <= 0 || MethodName.length <= 0) {
        return nil;
    }
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(moduleName);
    lua_pushstring(L, [MethodName cStringUsingEncoding:NSUTF8StringEncoding]);
    lua_rawget(L, -2);
    id ret = nil;
    if (lua_isfunction(L, -1)) {
        oc_fromObjc(L, params1);
        oc_fromObjc(L, params2);
        oc_fromObjc(L, params3);
        oc_fromObjc(L, params4);
        int err = lua_pcall(L, 4, 1, 0);
        if (err != 0) {
            luaError(L,"call_lua_function call error");
        } else {
            ret = oc_copyToObjc(L,-1);
        }
    } else {
        luaError(L,"call_lua_function call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    return ret;
}

id call_lua_function(NSString * moduleName , NSString * MethodName , id params1, id params2, id params3, id params4, id params5)
{
    if (moduleName.length <= 0 || MethodName.length <= 0) {
        return nil;
    }
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(moduleName);
    lua_pushstring(L, [MethodName cStringUsingEncoding:NSUTF8StringEncoding]);
    lua_rawget(L, -2);
    id ret = nil;
    if (lua_isfunction(L, -1)) {
        oc_fromObjc(L, params1);
        oc_fromObjc(L, params2);
        oc_fromObjc(L, params3);
        oc_fromObjc(L, params4);
        oc_fromObjc(L, params5);
        int err = lua_pcall(L, 5, 1, 0);
        if (err != 0) {
            luaError(L,"call_lua_function call error");
        } else {
            ret = oc_copyToObjc(L,-1);
        }
    } else {
        luaError(L,"call_lua_function call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    return ret;
}
