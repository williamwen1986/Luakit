# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

if (${ANDROID})
    set (SRC lua_timer_android.cpp)
else ()
    set (SRC lua_timer.cpp)
endif ()

add_library(
                # Sets the name of the library.
                timer

                # Sets the library as a shared library.
                STATIC

                # Provides a relative path to your source file(s).
                ${SRC}
)



# Specifies a path to native header files.
include_directories(
		    ${PROJECT_SOURCE_DIR}
            ${PROJECT_SOURCE_DIR}/..
		    ${PROJECT_SOURCE_DIR}/../../third-party/lua-5.1.5/src
		    ${PROJECT_SOURCE_DIR}/../../src
		    ${PROJECT_SOURCE_DIR}/../../include
		    ${PROJECT_SOURCE_DIR}/../../third-party
		    ${PROJECT_SOURCE_DIR}/../../src/common
		    ${PROJECT_SOURCE_DIR}/../../src/lua-tools
		    ${PROJECT_SOURCE_DIR}/../../AndroidFramework/jni
 )
