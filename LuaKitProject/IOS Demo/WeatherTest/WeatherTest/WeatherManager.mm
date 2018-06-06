//
//  WeatherManager.m
//  Luakit
//
//  Created by williamwen on 2018/3/21.
//  Copyright © 2018年 williamwen. All rights reserved.
//

#import "WeatherManager.h"
#include "common/business_runtime.h"
#include "tools/lua_helpers.h"
#import "oc_helpers.h"
#import "lauxlib.h"

@implementation WeatherManager

+ (NSArray *)getWeather {
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    [self pushLuaObject];
    lua_pushstring(L, "getWeather");
    lua_rawget(L, -2);
    NSArray *ret = nil;
    if (lua_isfunction(L, -1)) {
        int err = lua_pcall(L, 0, 1, 0);
        if (err != 0) {
            luaError(L,"getWeather call error");
        } else {
            ret = (NSArray *)oc_copyToObjc(L,-1);
        }
    } else {
         luaError(L,"getWeather call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    return ret;
}

+(void)pushLuaObject
{
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    NSString * lua = [NSString stringWithFormat:@"TEM_OC_OBJECT = require('%@')",NSStringFromClass([self class])];
    doString(L, [lua cStringUsingEncoding:NSUTF8StringEncoding]);
    lua_getglobal(L, "TEM_OC_OBJECT");
    lua_pushnil(L);
    lua_setglobal(L, "TEM_OC_OBJECT");
    END_STACK_MODIFY(L, 1)
}

+ (void)loadWeather:(void (^)(NSArray *)) callback
{
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    [self pushLuaObject];
    lua_pushstring(L, "loadWeather");
    lua_rawget(L, -2);
    if (lua_isfunction(L, -1)) {
        oc_fromObjc(L, callback);
        int err = lua_pcall(L, 1, 0, 0);
        if (err != 0) {
            luaError(L,"loadWeather call error");
        }
    } else {
        luaError(L,"loadWeather call error no such function");
    }
    END_STACK_MODIFY(L, 0)
}

@end
