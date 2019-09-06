Create a new project with xcode
-----------------------------
If you have your own project , skip this step

Copy the src code and framework
-----------------------------
Copy [source code folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/src) and [IOS framework folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/IOSFrameWork) to the rootPath of your project

Add dependence
-----------------------------
Open your app project, open Luakit/LuaKitProject/IOSFrameWork folder, drag the Luakit.xcodeproj to your project.

Go to Build Settings and Add Header Search Paths as below

```	
$(SRCROOT)/src/Projects
$(SRCROOT)/src/Projects/lua-5.1.5/lua
$(SRCROOT)/src/Projects/common
$(SRCROOT)/IOSFrameWork/Luakit/OCHelper
```

Go Build Phases -> Target Dependencies -> add Luakit

Go Build Phases -> Link Binary With Libraries -> add libLuakit.a , libz.tbd , libssl.a and libcrypto.a

libssl.a and libcrypto.a locate in [the folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/src/Projects/openssl/lib) src/Projects/openssl/lib/ 

Add the lua source to your project
-----------------------------
Our demo lua source code is in the [luaSrc folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/src/Projects/LuaSrc), src/Projects/LuaSrc

Initialization Luakit
-----------------------------
Add below code to your entrance of your app. In most cases , you can do this in main function, modify your main file name from main.m to main.mm and add below code

```c++
#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#include "base/command_line.h"
#include "base/memory/scoped_ptr.h"
#include "common/business_main_delegate.h"
#include "common/business_runtime.h"
#include "tools/lua_helpers.h"
#include "base/thread_task_runner_handle.h"
#include "common/base_lambda_support.h"
extern "C" {
#include "lua_h"
#include "lualib.h"
#include "lauxlib.h"
}

int main(int argc, char * argv[]) {
	CommandLine::Init(argc, argv);
	NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
	luaSetPackagePath([bundlePath cStringUsingEncoding:NSUTF8StringEncoding]);
	setXXTEAKeyAndSign("2dxLua", strlen("2dxLua"), "XXTEA", strlen("XXTEA"));
	scoped_ptr<BusinessMainDelegate> delegate(new BusinessMainDelegate());
	scoped_ptr<BusinessRuntime> business_runtime(BusinessRuntime::Create());
	business_runtime->Initialize(delegate.get());
	business_runtime->Run();
	@autoreleasepool {
	    return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
	}
}
```

Create your own business model
-----------------------------
Refer to [Demo code](https://github.com/williamwen1986/Luakit/blob/master/LuaKitProject/IOS%20Demo/WeatherTest/WeatherTest/WeatherManager.mm), WeatherManager.mm  demonstrate how to connect IOS and lua, Luakit provide a general interface to connect IOS and lua

```	objective-c
+ (NSArray *)getWeather {
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
