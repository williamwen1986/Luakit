//
//  WeatherManager.m
//  Luakit
//
//  Created by williamwen on 2018/3/21.
//  Copyright © 2018年 williamwen. All rights reserved.
//

#import "WeatherManager.h"
#include "common/business_runtime.h"
#include "lua-tools/lua_helpers.h"
#import "oc_helpers.h"
#import "lauxlib.h"

@implementation WeatherManager

+ (NSArray *)getWeather {
    return call_lua_function(@"WeatherManager", @"getWeather");
}

+ (void)loadWeather:(void (^)(NSArray *)) callback
{
    call_lua_function(@"WeatherManager", @"loadWeather", callback);
}

@end
