//
//  main.m
//  AsyncSocketTest
//
//  Created by larpoux on 12/09/2019.
//  Copyright © 2019 larpoux. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"
#import "oc_helpers.h"
#include "tools/lua_helpers.h"



int main(int argc, const char * argv[])
{
    startLuakit(argc, (char**)argv);
    //NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
    //luaSetPackagePath([bundlePath cStringUsingEncoding:NSUTF8StringEncoding]);

    return NSApplicationMain(argc, argv);
}
