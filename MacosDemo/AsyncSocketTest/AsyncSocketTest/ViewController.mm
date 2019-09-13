//
//  ViewController.m
//  AsyncSocketTest
//
//  Created by larpoux on 12/09/2019.
//  Copyright © 2019 larpoux. All rights reserved.
//

#import "ViewController.h"
#import "oc_helpers.h"


@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Do any additional setup after loading the view.
}


- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}


- (IBAction)TestButton:(id)sender
{
    lua_State * state = getCurrentThreadLuaState();
    //luaL_dostring(state, "package.path = '/MacosExt/Users/larpoux/?.lua;?;?.lua;Resources/?.lua;./Contents/Resources/?.lua;./Contents/Resources/?;/tmp/?.lua;;' ");
    luaL_dostring(state, "require('async_socket_test').test()");
}


@end
