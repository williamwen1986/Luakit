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

file(GLOB ALL_SRCS
        "${PROJECT_SOURCE_DIR}/common/*.cpp"
        "${PROJECT_SOURCE_DIR}/common/*.c"
        "${PROJECT_SOURCE_DIR}/common/*.cc"

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
                common

                # Sets the library as a shared library.
                STATIC

                ${ALL_SRCS}
)

#set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../curl/outputs/${ANDROID_ABI})
#file(MAKE_DIRECTORY ${lib_build_DIR})
#add_subdirectory( "${PROJECT_SOURCE_DIR}/../curl" ${lib_build_DIR} )
#set_target_properties( curl PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libcurl.a)

#set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../base/outputs/${ANDROID_ABI})
#file(MAKE_DIRECTORY ${lib_build_DIR})
#add_subdirectory( ${PROJECT_SOURCE_DIR}/../base ${lib_build_DIR} )
#set_target_properties( base    PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libbase.a)

#set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../libxml/outputs/${ANDROID_ABI})
#file(MAKE_DIRECTORY ${lib_build_DIR})
#add_subdirectory( ${PROJECT_SOURCE_DIR}/../libxml ${lib_build_DIR} )
#set_target_properties( xml PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libxml.a)

#target_link_libraries(common curl base xml)



# Specifies a path to native header files.
include_directories(
                    ${PROJECT_SOURCE_DIR}/../
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/../lua-5.1.5/lua
                    ${PROJECT_SOURCE_DIR}/../openssl/include/
                    ${PROJECT_SOURCE_DIR}/../curl/include
                    ${PROJECT_SOURCE_DIR}/../qlite-amalgamation-3290000
                    ${PROJECT_SOURCE_DIR}/../extensions/HTTP
                    ${PROJECT_SOURCE_DIR}/../extensions/lua-cjson-master
                    ${PROJECT_SOURCE_DIR}/../extensions/thread
                    ${PROJECT_SOURCE_DIR}/../extensions/timer
                    ${PROJECT_SOURCE_DIR}/../extensions/AsyncSocket
                    ${PROJECT_SOURCE_DIR}/../extensions/File
                    ${PROJECT_SOURCE_DIR}/../extensions/debug
                    ${PROJECT_SOURCE_DIR}/../extensions/luasocket
                    ${PROJECT_SOURCE_DIR}/../extensions/Notify
                    ${PROJECT_SOURCE_DIR}/../extensions/language
                    ${PROJECT_SOURCE_DIR}/../jni
                    ${PROJECT_SOURCE_DIR}/../lua-5.1.5/lua/tools
                    ${PROJECT_SOURCE_DIR}/../sqlite-amalgamation-3290000
)
