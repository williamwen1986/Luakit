
Build the OpenSSL library
-------------------------
You need to build the openSSL library for your target API version, such as:

```sh
export CONFIG=Debug
cd luakit/src/openssl-1.1.1c/
./build-ios.sh
```

The $CONFIG environment variable must be "Debug" or "Release".

You will get your library in luakit/libs/


Build the Luakit library (optional)
-----------------------------------
If you want to build the complete Luakit library and not just Open SSL, you can do the following instead of the previous step :

```sh
export CONFIG=Debug
export ANDROID_API=24
cd luakit/
./build-android.sh
```

The $CONFIG environment variable must be "Debug" or "Release".

You will get your library in luakit/libs/

Create a new project with xcode
-------------------------------
If you have your own project , skip this step


Copy the src code and framework
-------------------------------
Copy Github [source code folder](../../../..) somewhere on your disk.

Add dependences
---------------
Open your app project,  drag and drop the luakit/IOSFrameWork/Luakit.xcodeproj from the finder to your project.

Got to Build Settings and add a User-Defined variable to your luakit source folder.
For example:
```
LUAKIT_ROOT = $(SRCROOT)/../..
```

Go to Build Settings and Add Header Search Paths as below

```
$(LUAKIT_ROOT)/src
$(LUAKIT_ROOT)/src/lua-5.1.5/lua
$(LUAKIT_ROOT)/src/common
$(LUAKIT_ROOT)/IOSFrameWork/Luakit/OCHelper
```

Go to Build Settings and add Library Search Paths as below:
```
$(LUAKIT_ROOT)/libs/ios$SDK_VERSION-$CONFIGURATION
```

Go Build Phases -> Target Dependencies -> add Luakit

Go Build Phases -> Link Binary With Libraries -> add libLuakit.a and libssl.a (libssl.a is located in the sub-folder luakit/libs/...)

Add the lua source to your project
----------------------------------
Go to Build Phases, Copy Bundle Resources, and add your "lua" directory. The name "lua" is important. You must check the option "Create folder Refeferences"

Our demo lua source code is in the [luaSrc folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/src/Projects/LuaSrc), src/Projects/LuaSrc

Initialization Luakit
---------------------
Add below code to your entrance of your app. In most cases , you can do this in main function.

 __Modify your main file name from main.m to main.mm and add below code__

```c++
#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#import "oc_helpers.h"
int main(int argc, char * argv[])
{
    startLuakit(argc, argv);
    lua_State * state = getCurrentThreadLuaState();
    luaL_dostring(state, "require('notification_test').test()");
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
```

Create your own business model
-----------------------------
Refer to [Demo code](https://github.com/williamwen1986/Luakit/blob/master/LuaKitProject/IOS%20Demo/WeatherTest/WeatherTest/WeatherManager.mm), WeatherManager.mm  demonstrate how to connect IOS and lua, Luakit provide a general interface to connect IOS and lua

```objective-c
#import "WeatherManager.h"
#include "common/business_runtime.h"
#include "tools/lua_helpers.h"
#import "oc_helpers.h"
#import "lauxlib.h"

@implementation WeatherManager

+ (NSArray *)getWeather
{
    return call_lua_function(@"WeatherManager", @"getWeather");
}

+ (void)loadWeather:(void (^)(NSArray *)) callback
{
    call_lua_function(@"WeatherManager", @"loadWeather", callback);
}
```

The goal of the above code is to connect the [lua file](https://github.com/williamwen1986/Luakit/blob/master/LuaKitProject/src/Projects/LuaSrc/WeatherManager.lua) , the lua code is the finally working code

```lua
local _weatherManager = {}

local Table = require('orm.class.table')
local _weatherTable = Table("weather")

_weatherManager.getWeather = function ()
	return _weatherTable.get:all():getPureData()
end

_weatherManager.parseWeathers = function (responseStr,callback)
	local t = cjson.decode(responseStr)
	local weatherTable = _weatherTable
	local ret = {}
	if t and t.weather and t.weather[1] and t.weather[1].future then
		weatherTable.get:delete()
		local city = t.weather[1].city_name
		for i,v in ipairs(t.weather[1].future) do
			local t = {}
			t.wind = v.wind
			t.date = v.date
			t.low = tonumber(v.low)
			t.high = tonumber(v.high)
			t.id = i
			t.city = city
			local weather = weatherTable(t)
			weather:save()
			table.insert(ret,weather:getPureData())
		end
	end
	if callback then
		callback(ret)
	end
end

_weatherManager.loadWeather = function (callback)
	lua_http.request({ url  = "http://tj.nineton.cn/Heart/index/all?city=CHSH000000",
		onResponse = function (response)
			if response.http_code ~= 200 then
				if callback then
					callback(nil)
				end
			else
				lua_thread.postToThread(BusinessThreadLOGIC,"WeatherManager","parseWeathers",response.response,function(data)
					if callback then
						callback(data)
					end
				end)
			end
		end})
end

return _weatherManager
```
