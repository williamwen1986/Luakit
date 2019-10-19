# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

file(GLOB ALL_SRCS
	"${PROJECT_SOURCE_DIR}/*.cpp"
	"${PROJECT_SOURCE_DIR}/*.c"
	"${PROJECT_SOURCE_DIR}/*.cc"

	"${PROJECT_SOURCE_DIR}/net/*.cpp"
	"${PROJECT_SOURCE_DIR}/net/*.c"
	"${PROJECT_SOURCE_DIR}/net/*.cc"
)
list(FILTER ALL_SRCS EXCLUDE REGEX ".*_win.cc$")
 

add_library(
                # Sets the name of the library.
                network

                # Sets the library as a shared library.
                STATIC

                # Provides a relative path to your source file(s).
                ${ALL_SRCS}
)

# Specifies a path to native header files.
include_directories(
                ${PROJECT_SOURCE_DIR}
                ${PROJECT_SOURCE_DIR}/../Notify
                ${PROJECT_SOURCE_DIR}/../../src
                ${PROJECT_SOURCE_DIR}/../../src/common
                ${PROJECT_SOURCE_DIR}/../../src/base
                ${PROJECT_SOURCE_DIR}/../../src/notification
                ${PROJECT_SOURCE_DIR}/../../third-party/lua-5.1.5/src
                ${PROJECT_SOURCE_DIR}/../../src/lua-tools
                ${PROJECT_SOURCE_DIR}/../../third-party/curl/include
                ${PROJECT_SOURCE_DIR}/../../include
                ${PROJECT_SOURCE_DIR}/../../third-party
)
