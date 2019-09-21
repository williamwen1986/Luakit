
Build the OpenSSL library
-------------------------
You need to build the openSSL library for your target API version, such as:

```sh
export CONFIG=Debug
<<<<<<< HEAD
export IOS_SDK_VERSION=12.2
=======
>>>>>>> Merge "build-macos" branch with William
cd luakit/src/openssl-1.1.1c/
./build-ios.sh
```

The $CONFIG environment variable must be "Debug" or "Release".
<<<<<<< HEAD
The $IOS_SDK_VERSION is your target SDK.
=======
>>>>>>> Merge "build-macos" branch with William

You will get your library in luakit/libs/


Build the Luakit library (optional)
-----------------------------------
If you want to build the complete Luakit library and not just Open SSL, you can do the following instead of the previous step :

```sh
export CONFIG=Debug
<<<<<<< HEAD
export IOS_SDK_VERSION=12.2
=======
>>>>>>> Merge "build-macos" branch with William
cd luakit/
./build-ios.sh
```

The $CONFIG environment variable must be "Debug" or "Release".
<<<<<<< HEAD
The $IOS_SDK_VERSION is your target SDK.
=======
>>>>>>> Merge "build-macos" branch with William

You will get your library in luakit/libs/

Create a new project with xcode
-------------------------------
If you have your own project , skip this step


Copy the src code and framework
-------------------------------
Copy Github [source code folder](../../..) somewhere on your disk.

Add dependences
---------------
Open your app project,  drag and drop the luakit/IOSFrameWork/Luakit.xcodeproj from the finder to your project.

Go to Build Settings and add a User-Defined variable to your luakit source folder.
For example:
```
LUAKIT_ROOT = $(SRCROOT)/../..
```

Go to Build Settings and Add Header Search Paths as below

```
$(LUAKIT_ROOT)/src
<<<<<<< HEAD
$(LUAKIT_ROOT)/src/lua-5.1.5/src
$(LUAKIT_ROOT)/src/common
$(LUAKIT_ROOT)/IOSFrameWork/Luakit/OCHelper$(LUAKIT_ROOT)
$(LUAKIT_ROOT)/config
$(LUAKIT_ROOT)
$(LUAKIT_ROOT)/third-party
=======
$(LUAKIT_ROOT)/src/lua-5.1.5/lua
$(LUAKIT_ROOT)/src/common
$(LUAKIT_ROOT)/IOSFrameWork/Luakit/OCHelper
>>>>>>> Merge "build-macos" branch with William
```

Go to Build Settings and add Library Search Paths as below:
```
$(LUAKIT_ROOT)/libs/ios$SDK_VERSION-$CONFIGURATION
```

Go Build Phases -> Target Dependencies -> add Luakit

Go Build Phases -> Link Binary With Libraries -> add libLuakit.a and libssl.a (libssl.a is located in the sub-folder luakit/libs/...)

<<<<<<< HEAD
Add the lua sources to your project
-----------------------------------
If you have lua files, go to Build Phases, Copy Bundle Resources, and add your "lua" directory. The name "lua" is important. You must check the option "Create folder Refeferences"
=======
Add the lua source to your project
----------------------------------
Go to Build Phases, Copy Bundle Resources, and add your "lua" directory. The name "lua" is important. You must check the option "Create folder Refeferences"
>>>>>>> Merge "build-macos" branch with William

Our demo lua source code is in the [luaSrc sub-folder](../IOSDemo)

Initialization Luakit
---------------------
<<<<<<< HEAD

Add below code to your entrance of your app. In most cases , you can do this in main function.
Modify your main file name from main.m to main.mm and add code something like below :
=======
Add below code to your entrance of your app. In most cases , you can do this in main function.

 _Initialization Luakit
---------------------
Add below code to your entrance of your app. In most cases , you can do this in main function.

 Modify your main file name from main.m to main.mm and add code something like below :
>>>>>>> Merge "build-macos" branch with William

```c++
#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#import "oc_helpers.h"
int main(int argc, char * argv[])
{
    startLuakit(argc, argv);
    lua_State * state = getCurrentThreadLuaState();
    luaL_dostring(state, "require('notification_test').test()");
    return NSApplicationMain(argc, (const char**)argv);
    }
}
```
