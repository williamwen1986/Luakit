//
//  main.m
//  ThreadTest
//
//  Created by larpoux on 16/09/2019.
//  Copyright © 2019 larpoux. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"
#import "oc_helpers.h"
#include "tools/lua_helpers.h"


int main(int argc, const char * argv[])
{
    startLuakit(argc, (char**)argv);
    return NSApplicationMain(argc, argv);
}
