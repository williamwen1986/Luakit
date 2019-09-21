//
//  main.m
//  WeatherTest
//
//  Created by larpoux on 16/09/2019.
//  Copyright © 2019 larpoux. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"
#import "oc_helpers.h"
#include "tools/lua_helpers.h"


void luaError (const char * info)
{
    if (info != NULL)
    {
        NSLog(@"%@",[NSString stringWithUTF8String:info]);
    }
}


int main(int argc, const char * argv[])
{
    setLuaError(luaError);
    startLuakit(argc, (char**)argv);
    return NSApplicationMain(argc, argv);
}
