# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_HISTORICAL_STRERRO_R -DNVALGRIND -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti -fPIC -DUSE_HISTORICAL_STRERRO_R")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -llog -lz -lc")


file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/lua/*.c"
        "${PROJECT_SOURCE_DIR}/lua/*.h"
        "${PROJECT_SOURCE_DIR}/lua/tools/*.cpp"
        "${PROJECT_SOURCE_DIR}/lua/tools/*.h"
)

add_library(
                # Sets the name of the library.
                lua

                # Sets the library as a shared library.
                STATIC

                # Provides a relative path to your source file(s).
                ${all_SRCS}
)

# Specifies a path to native header files.
include_directories(
                ${PROJECT_SOURCE_DIR}/lua
                ${PROJECT_SOURCE_DIR}/../lua-5.1.5/lua/tools
                ${PROJECT_SOURCE_DIR}/../
                ${PROJECT_SOURCE_DIR}/../common/
                ${PROJECT_SOURCE_DIR}/../sqlite-amalgamation-3290000
                ${PROJECT_SOURCE_DIR}/../extensions/lua-cjson-master
                ${PROJECT_SOURCE_DIR}/../extensions/debug
                ${PROJECT_SOURCE_DIR}/../extensions/HTTP
                ${PROJECT_SOURCE_DIR}/../extensions/AsyncSocket
                ${PROJECT_SOURCE_DIR}/../extensions/timer
                ${PROJECT_SOURCE_DIR}/../extensions/thread
                ${PROJECT_SOURCE_DIR}/../extensions/File
                ${PROJECT_SOURCE_DIR}/../extensions/Notify
                ${PROJECT_SOURCE_DIR}/../extensions/language
)
