Create a new project with Android Studio
-----------------------------
If you have your own project , skip this step

Copy the src code and framework
-----------------------------
Copy [source code folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/src) and [android framework folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/AndroidFrameWork) to the rootPath of your project

Add dependence
-----------------------------
Open your project, add jniLibs.srcDir to your app build.gradle, such as below

```	
apply plugin: 'com.android.application'

android {
    compileSdkVersion 26
    defaultConfig {
		...
    }
    buildTypes {
       ...
    }
    //add jniLibs.srcDir
    sourceSets{
        main{
            jniLibs.srcDir '../src/Projects/jni/libs'
        }
    }
}
```


Add two modules in [AndroidFrameWork folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/AndroidFrameWork) [lib_chromium](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/AndroidFrameWork/lib_chromium) and [luakit](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/AndroidFrameWork/luakit) to your project. 

File->New->Import Module

Modify Project stucture to add dependence

File->Project Stucture->select your app->Dependencies-> + -> Module Dependency ->  select lib_chromium and luakit

Copy your lua source code to android assets/lua folder
-----------------------------
Our demo lua source code is in the [luaSrc folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/src/Projects/LuaSrc), you need to copy the source code your need to assets/lua folder, you can also add your own lua file to assets/lua folder.

Initialization Luakit
-----------------------------
Add below code to your entrance of your app

```java
LuaHelper.startLuaKit(this);
```
Create your own business model
-----------------------------
Luakit provide general interface to connect java and lua ,Refer to [LuaHelper.java](https://github.com/williamwen1986/Luakit/blob/master/LuaKitProject/AndroidFrameWork/luakit/src/main/java/com/common/luakit/LuaHelper.java.java) 

```java
Object[] ret =  (Object[]) LuaHelper.callLuaFunction("WeatherManager","getWeather");

ILuaCallback callback = new ILuaCallback() {
    @Override
    public void onResult(Object o) {
        Object[] ret = (Object[])o;
        adapter.source = ret;
        adapter.notifyDataSetChanged();
    }
};

LuaHelper.callLuaFunction("WeatherManager","loadWeather", callback);
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
Maintain your own jni project
-----------------------------

Luakit is provide a integrated jni project in [the folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/src/Projects/jni), you can go to this path in the console, and type the below command. Luakit is compile with [android-ndk-r16b](https://developer.android.com/ndk/downloads/older_releases#ndk-16b-downloads)

```
ndk-build clean  NDK_PROJECT_PATH=. NDK_APPLICATION_MK=Application.mk NDK_MODULE_PATH=../../Projects

ndk-build  NDK_PROJECT_PATH=. NDK_APPLICATION_MK=Application.mk NDK_MODULE_PATH=../../Projects 

```
