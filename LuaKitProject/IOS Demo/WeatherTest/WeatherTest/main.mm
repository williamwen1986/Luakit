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

int main(int argc, char * argv[]) {
    startLuakit(argc, argv);
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
