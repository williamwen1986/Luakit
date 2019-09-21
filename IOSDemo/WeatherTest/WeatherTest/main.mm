//
//  main.m
//  WeatherTest
//
//  Created by williamwen on 2018/5/31.
//  Copyright © 2018年 williamwen. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#import "oc_helpers.h"

void luaError (const char * info)
{
    if (info != NULL) {
        NSLog(@"%@",[NSString stringWithUTF8String:info]);
    }
}

int main(int argc, char * argv[]) {
    setLuaError(luaError);
    startLuakit(argc, argv);
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
