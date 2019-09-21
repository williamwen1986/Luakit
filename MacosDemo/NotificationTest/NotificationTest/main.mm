//
//  main.m
//  NotificationTest
//
//  Created by larpoux on 11/09/2019.
//  Copyright © 2019 larpoux. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"
#import "oc_helpers.h"

int main(int argc, char * argv[])
{
    startLuakit(argc, argv);
    //lua_State * state = getCurrentThreadLuaState();
    //luaL_dostring(state, "require('notification_test').test()");
    return NSApplicationMain(argc, (const char**)argv);
}
