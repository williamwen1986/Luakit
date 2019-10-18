Prerequisite
-----------------------------
You can develop your application, using Android Studio on :
- Linux
- Macos
- Windows, using WSL (Windows subsystem for Linux)

You must install :
* Android Studio
* Cmake
* The Android SDKs
* The Android NDK

Note : Windows users must install a Linux subsystem (WSL) like Ubuntu or Kali. They will also have to install the Android NDK onto this subsystem (A Linux NDK). It means that Windows users will need to have two NDKs. One on Linux, and one on Windows. C'est la vie.

Now, set the needed environment variables. In your ~/.bashrc add the following two variables :
```sh
export ANDROID_HOME=...
export ANDROID_NDK_HOME=...
```

Build the OpenSSL library
-------------------------
You need to build the openSSL library for your target API version, such as:

```sh
export CONFIG=Debug
export ANDROID_API=24
cd luakit/src/openssl-1.1.1c/
./build-android.sh
```

The $CONFIG environment variable must be "Debug" or "Release".
The $ANDROID_API environment variable is your target API version.

You will get your library in luakit/libs/


Build the Luakit library (optional)
-----------------------------------
If you want to build the complete Luakit library (libluaFramework.so) and not just Open SSL, you can do the following instead of the previous step :

```sh
export CONFIG=Debug
export ANDROID_API=24
cd luakit/
./build-android.sh
```

The $CONFIG environment variable must be "Debug" or "Release".
The $ANDROID_API environment variable is your target API version.

You will get your library in luakit/libs/


Create a new project with Android Studio
----------------------------------------
If you have your own project , skip this step


Copy the src code and framework
-------------------------------
Copy (or clone) Github [source code folder](../..) somewhere on your disk.

Add dependence
--------------
- Create a CMakeLists.txt file in your project :
```
cmake_minimum_required(VERSION 3.4.1)
project(luakit)
set( LUAKIT_ROOT ${PROJECT_SOURCE_DIR}/../../.. )
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -llog -lz -lc")
file(GLOB jni_SRCS "${LUAKIT_ROOT}/AndroidFramework/jni/*.cpp")

add_library( luakitApp SHARED LuakitExtensions.cpp ${jni_SRCS} )

set( lib_build_DIR ${LUAKIT_ROOT}/AndroidFramework/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${LUAKIT_ROOT}/AndroidFramework"  ${lib_build_DIR})
set_target_properties( luaFramework PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libluaFramework.a)


target_link_libraries( luakitApp luaFramework )

include_directories(
                    ${LUAKIT_ROOT}/AndroidFramework
                    ${LUAKIT_ROOT}/AndroidFramework/jni
                    ${LUAKIT_ROOT}/src
                    ${LUAKIT_ROOT}/src/common
                    ${LUAKIT_ROOT}/include
                    ${LUAKIT_ROOT}/third-party
                    ${LUAKIT_ROOT}/third-party/lua-5.1.5/src
                    ${LUAKIT_ROOT}/src/lua-tools
)

```
- Open your project, add jniLibs.srcDir to your app build.gradle, such as below


```
apply plugin: 'com.android.application'

android {
    compileSdkVersion 28
    defaultConfig {
                minSdkVersion 24
                targetSdkVersion 28
		...
    }
    buildTypes {
       ...
    }

    externalNativeBuild {
        cmake {
            version "3.10.2"
            path file('./CMakeLists.txt')
        }
    }

    dependencies {
        implementation 'com.android.support:appcompat-v7:28.0.0'
        implementation 'com.android.support.constraint:constraint-layout:1.1.3'
           ...
        implementation project(':luakit')
        //implementation 'com.github.williamwen1986:LuakitJitpack:1.0.9'

    }
}
```

Create a "setting.gradle" file into your Project Directory, with the following :

```
include ':app', ':luakit', ':lib_chromium'
project(':luakit').projectDir = new File(settingsDir, '../../AndroidFramework/luakit')
project(':lib_chromium').projectDir = new File(settingsDir, '../../AndroidFrameWork/lib_chromium')
```

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

Copy your lua source code to android assets/lua folder
------------------------------------------------------
- Go to Build Phases -> Copy Bundle Resources, and add your "lua" directory. The name "lua" is important. You must check the option "Create folder Refeferences"
- If you want a Luakit Extension which needs Lua sources, go to Build Phases, Copy Bundle Resources, and add your "lua-extension" directory. The name "lua-extension" is important. You must check the option "Create folder Refeferences"

Initialization Luakit
---------------------
Add below code to your entrance of your app

```java
LuaHelper.startLuaKit(this);
```

Create your own business model
------------------------------
Luakit provide general interface to connect java and lua ,Refer to [LuaHelper.java](../src/main/java/com/common/luakit/LuaHelper.java.java)

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
The goal of the above code is to connect the [lua file](../AndroidDemo/WeatherTest/app/src/main/assets/lua/WeatherManager.lua).


