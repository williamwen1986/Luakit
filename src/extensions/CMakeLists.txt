# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

file(GLOB ALL_SRCS
 
	"${PROJECT_SOURCE_DIR}/AsyncSocket/*.cpp"
	"${PROJECT_SOURCE_DIR}/AsyncSocket/*.c"
	"${PROJECT_SOURCE_DIR}/AsyncSocket/*.cc"

	"${PROJECT_SOURCE_DIR}/debug/*.cpp"
	"${PROJECT_SOURCE_DIR}/debug/*.c"
	"${PROJECT_SOURCE_DIR}/debug/*.cc"

	"${PROJECT_SOURCE_DIR}/File/*.cpp"
	"${PROJECT_SOURCE_DIR}/File/*.c"
	"${PROJECT_SOURCE_DIR}/File/*.cc"

	"${PROJECT_SOURCE_DIR}/HTTP/*.cpp"
	"${PROJECT_SOURCE_DIR}/HTTP/*.c"
	"${PROJECT_SOURCE_DIR}/HTTP/*.cc"

	"${PROJECT_SOURCE_DIR}/language/*.cpp"
	"${PROJECT_SOURCE_DIR}/language/*.c"
	"${PROJECT_SOURCE_DIR}/language/*.cc"

	"${PROJECT_SOURCE_DIR}/lua-cjson-master/*.cpp"
	"${PROJECT_SOURCE_DIR}/lua-cjson-master/*.c"
	"${PROJECT_SOURCE_DIR}/lua-cjson-master/*.cc"

	"${PROJECT_SOURCE_DIR}/luasocket/*.cpp"
	"${PROJECT_SOURCE_DIR}/luasocket/*.c"
	"${PROJECT_SOURCE_DIR}/luasocket/*.cc"

	"${PROJECT_SOURCE_DIR}/Notify/*.cpp"
	"${PROJECT_SOURCE_DIR}/Notify/*.c"
	"${PROJECT_SOURCE_DIR}/Notify/*.cc"

	"${PROJECT_SOURCE_DIR}/thread/*.cpp"
	"${PROJECT_SOURCE_DIR}/thread/*.c"
	"${PROJECT_SOURCE_DIR}/thread/*.cc"

	"${PROJECT_SOURCE_DIR}/timer/*.cpp"
	"${PROJECT_SOURCE_DIR}/timer/*.c"
	"${PROJECT_SOURCE_DIR}/timer/*.cc"

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
                extensions

                # Sets the library as a shared library.
                STATIC

                ${ALL_SRCS}
)

# Specifies a path to native header files.
include_directories(
                    ${PROJECT_SOURCE_DIR}/..
                    ${PROJECT_SOURCE_DIR}/../common/
                    ${PROJECT_SOURCE_DIR}/lua-cjson-master
                    ${PROJECT_SOURCE_DIR}/debug
                    ${PROJECT_SOURCE_DIR}/HTTP
                    ${PROJECT_SOURCE_DIR}/AsyncSocket
                    ${PROJECT_SOURCE_DIR}/timer
                    ${PROJECT_SOURCE_DIR}/thread
                    ${PROJECT_SOURCE_DIR}/luasocket
                    ${PROJECT_SOURCE_DIR}/File
                    ${PROJECT_SOURCE_DIR}/Notify
                    ${PROJECT_SOURCE_DIR}/../lua-5.1.5/lua
                    ${PROJECT_SOURCE_DIR}/../lua-5.1.5/lua/tools
                    ${PROJECT_SOURCE_DIR}/../curl/include
		    ${PROJECT_SOURCE_DIR}/../jni
		    ${PROJECT_SOURCE_DIR}/../network
		    ${PROJECT_SOURCE_DIR}/../network/net
)
