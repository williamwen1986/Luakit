# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

file(GLOB ALL_SRCS
 
	"${PROJECT_SOURCE_DIR}/*.cpp"
	"${PROJECT_SOURCE_DIR}/*.c"
	"${PROJECT_SOURCE_DIR}/*.cc"


)

list(FILTER ALL_SRCS EXCLUDE REGEX ".*lua_timer.cpp$")
list(FILTER ALL_SRCS EXCLUDE REGEX ".*_win.cc$")
list(FILTER ALL_SRCS EXCLUDE REGEX ".*shell.c$")
list(FILTER ALL_SRCS EXCLUDE REGEX ".*test.cpp$")
list(FILTER ALL_SRCS EXCLUDE REGEX ".*mock_server.cpp$")
list(FILTER ALL_SRCS EXCLUDE REGEX ".*tests.cpp$")
list(FILTER ALL_SRCS EXCLUDE REGEX ".*_ios.cpp$")
list(FILTER ALL_SRCS EXCLUDE REGEX ".*html_utils.cpp$")
list(FILTER ALL_SRCS EXCLUDE REGEX ".*wwhttpapi.cpp$")
list(FILTER ALL_SRCS EXCLUDE REGEX ".*authhttpapi.cpp$")

add_library(
                # Sets the name of the library.
                HTTP

                # Sets the library as a shared library.
                STATIC

                ${ALL_SRCS}
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
 )
