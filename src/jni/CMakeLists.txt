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
        "${PROJECT_SOURCE_DIR}/*.h"
        "${PROJECT_SOURCE_DIR}/*.hpp"
        "${PROJECT_SOURCE_DIR}/*.cpp"
        "${PROJECT_SOURCE_DIR}/*.c"
        )

list(FILTER all_SRCS EXCLUDE REGEX ".*lua_timer.cpp$")
list(FILTER all_SRCS EXCLUDE REGEX ".*shell.c$")



add_library( luaFramework SHARED ${all_SRCS})
link_directories(${PROJECT_SOURCE_DIR}/../../libs/android${ANDROID_NATIVE_API_LEVEL}-${CMAKE_BUILD_TYPE})

#---- set(outputDir $ENV{OUTPUT_DIR})

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../common/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../common" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libcommon.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../base/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../base" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libbase.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../libevent/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../libevent" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libevent.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../curl/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../curl" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libcurl.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../libxml/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../libxml" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libxml.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../modp_b64/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../modp_b64" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libmodp_b64.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../libiconv-1.14/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../libiconv-1.14" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libiconv.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../lua-5.1.5/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../lua-5.1.5" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/liblua.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../sqlite-amalgamation-3290000/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../sqlite-amalgamation-3290000" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libsqlite.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../extensions/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../extensions" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libextensions.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../network/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../network" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libnetwork.a)



add_library( ssl
             STATIC
             IMPORTED )

set_target_properties( # Specifies the target library.
                       ssl

                       # Specifies the parameter you want to define.
                       PROPERTIES IMPORTED_LOCATION

                      # Provides the path to the library you want to import.
                      ${PROJECT_SOURCE_DIR}/../../libs/android${ANDROID_NATIVE_API_LEVEL}-${CMAKE_BUILD_TYPE}/${ANDROID_ABI}/libssl.a 
                )

add_library( crypto
             STATIC
             IMPORTED )

set_target_properties( # Specifies the target library.
                       crypto

                       # Specifies the parameter you want to define.
                       PROPERTIES IMPORTED_LOCATION

                        #Provides the path to the library you want to import.
                        ${PROJECT_SOURCE_DIR}/../../libs/android${ANDROID_NATIVE_API_LEVEL}-${CMAKE_BUILD_TYPE}/${ANDROID_ABI}/libcrypto.a 
                      )

link_directories(${outputDir}/${ANDROID_ABI})
target_link_libraries(luaFramework  common  modp_b64 iconv lua sqlite extensions network curl base event ssl crypto xml )

# Specifies a path to native header files.
include_directories(
                    $ENV{OUTPUT_DIR}/
                    ${PROJECT_SOURCE_DIR}/../
		                ${PROJECT_SOURCE_DIR}/../jni
                    ${PROJECT_SOURCE_DIR}/../common/
                    ${PROJECT_SOURCE_DIR}/../openssl/include/
                    ${PROJECT_SOURCE_DIR}/../curl/include
                    ${PROJECT_SOURCE_DIR}/../lua-5.1.5/lua
                    ${PROJECT_SOURCE_DIR}/../lua-5.1.5/lua/tools
                    ${PROJECT_SOURCE_DIR}/../extensions/lua-cjson-master
                    ${PROJECT_SOURCE_DIR}/../extensions/debug
                    ${PROJECT_SOURCE_DIR}/../extensions/HTTP
                    ${PROJECT_SOURCE_DIR}/../extensions/AsyncSocket
                    ${PROJECT_SOURCE_DIR}/../extensions/timer
                    ${PROJECT_SOURCE_DIR}/../extensions/thread
                    ${PROJECT_SOURCE_DIR}/../extensions/File
                    ${PROJECT_SOURCE_DIR}/../extensions/Notify

)
