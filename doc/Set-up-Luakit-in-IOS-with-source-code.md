
Build the OpenSSL library
-------------------------
You need to build the openSSL library for your target API version, such as:

```sh
export CONFIG=Debug
export IOS_SDK_VERSION=12.2
cd luakit/third-party/openssl-1.1.1c/
./build-ios.sh
```

The $CONFIG environment variable must be "Debug" or "Release".
The $IOS_SDK_VERSION is your target SDK.

You will get your library in luakit/libs/


Build the Luakit library (optional)
-----------------------------------
If you want to build the complete Luakit library and not just Open SSL, you can do the following instead of the previous step :

```sh
export CONFIG=Debug
export IOS_SDK_VERSION=13.0
cd luakit/
./build-ios.sh
```

The $CONFIG environment variable must be "Debug" or "Release".
The $IOS_SDK_VERSION is your target SDK.

You will get your library in luakit/libs/

Create a new project with xcode
-------------------------------
If you have your own project , skip this step


Copy the src code and framework
-------------------------------
Copy (or clone) Github [source code folder](../..) somewhere on your disk.

Add dependences
---------------
- Open your app project,  drag and drop the luakit/AppleFramework/luakit.xcodeproj from the finder to your project.

- Go to Build Settings and add a User-Defined variable to your luakit source folder.
For example:
```
LUAKIT_ROOT = $(SRCROOT)/../..
```

- Go to Build Settings and Add Header Search Paths as below

```
$(LUAKIT_ROOT)
$(LUAKIT_ROOT)/include
$(LUAKIT_ROOT)/src
$(LUAKIT_ROOT)/third-party/lua-5.1.5/src
$(LUAKIT_ROOT)/src/common
$(LUAKIT_ROOT)/AppleFramework/OCHelper
$(LUAKIT_ROOT)/third-party
```

- Go to Build Settings and add Library Search Paths as below:
```
$(LUAKIT_ROOT)/libs/ios$SDK_VERSION-$CONFIGURATION
```

- Go to Build Phases -> Target Dependencies -> add Luakit
- Go to Build Phases -> Link Binary With Libraries -> add libLuakit.a
- If you need that openSSL to be linked with your project :
Go to Build Phases -> Link Binary With Libraries -> add libssl.a (libssl.a is located in the sub-folder luakit/libs/...)

Specify the Luakit Extensions that you need
-------------------------------------------
Create a ".cpp" file in your project as below :
```c++
extern class LuakitExtension TheThreadExtension;
extern class LuakitExtension TheTimerExtension;

extern class LuakitExtension* ExtensionsList [] =
{
        &TheThreadExtension,
        &TheTimerExtension,
        0
};
```


Add the lua sources to your project
-----------------------------------
- Go to Build Phases -> Copy Bundle Resources, and add your "lua" directory. The name "lua" is important. You must check the option "Create folder Refeferences"
- If you want a Luakit Extension which needs Lua sources, go to Build Phases, Copy Bundle Resources, and add your "lua-extension" directory. The name "lua-extension" is important. You must check the option "Create folder Refeferences"


Initialization Luakit
---------------------

Add below code to your entrance of your app. In most cases , you can do this in main function.
Modify your main file name from main.m to main.mm and add code something like below :

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
```
