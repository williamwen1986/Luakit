//
//  WeatherManager.h
//  Luakit
//
//  Created by williamwen on 2018/3/21.
//  Copyright © 2018年 williamwen. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface WeatherManager : NSObject

+(NSArray *)getWeather;

+(void)loadWeather:(void (^)(NSArray *)) callback;

@end
