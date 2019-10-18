# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_HISTORICAL_STRERRO_R -DNVALGRIND -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti -fPIC -DUSE_HISTORICAL_STRERRO_R")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -llog -lz -lc")



file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/*.cpp"
        "${PROJECT_SOURCE_DIR}/*.c"
        )

list(FILTER all_SRCS EXCLUDE REGEX ".*lua_timer.cpp$")
list(FILTER all_SRCS EXCLUDE REGEX ".*shell.c$")



add_library( jni 
        STATIC 
        ${all_SRCS}
)

# Specifies a path to native header files.
include_directories(
                    $ENV{OUTPUT_DIR}/
                    ${PROJECT_SOURCE_DIR}/../
                    ${PROJECT_SOURCE_DIR}/../../src/
                    ${PROJECT_SOURCE_DIR}/../../src/common/
                    ${PROJECT_SOURCE_DIR}/../../third-party
                    ${PROJECT_SOURCE_DIR}/../../third-party/openssl/include/
                    ${PROJECT_SOURCE_DIR}/../../third-party/curl/include
                    ${PROJECT_SOURCE_DIR}/../../third-party/lua-5.1.5/src
                    ${PROJECT_SOURCE_DIR}/../../src/lua-tools
                    ${PROJECT_SOURCE_DIR}/../../third-party/sqlite-amalgamation-3290000
                    ${PROJECT_SOURCE_DIR}/../../third-party/toluapp/include
                    ${PROJECT_SOURCE_DIR}/../../include

)
