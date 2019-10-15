# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

 
set( lib_build_DIR ${PROJECT_SOURCE_DIR}/AsyncSocket/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/AsyncSocket" ${lib_build_DIR} )
set_target_properties( AsyncSocket PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libAsyncSocket.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/debug/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/debug" ${lib_build_DIR} )
set_target_properties( debug PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libdebug.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/File/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/File" ${lib_build_DIR} )
set_target_properties( File PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libFile.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/HTTP/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/HTTP" ${lib_build_DIR} )
set_target_properties( HTTP PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libHTTP.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/language/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/language" ${lib_build_DIR} )
set_target_properties( language PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/liblanguage.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/lua-cjson-master/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/lua-cjson-master" ${lib_build_DIR} )
set_target_properties( lua_cjson PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/liblua_cjson.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/Notify/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/Notify" ${lib_build_DIR} )
set_target_properties( Notify PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libNotify.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/luasocket/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/luasocket" ${lib_build_DIR} )
set_target_properties( luasocket PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libluasocket.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/thread/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/thread" ${lib_build_DIR} )
set_target_properties( thread PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libthread.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/timer/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/timer" ${lib_build_DIR} )
set_target_properties( timer PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libttimer.a)



add_library(
                # Sets the name of the library.
                extensions

                # Sets the library as a shared library.
                STATIC

                dummy.c
)

target_link_libraries( extensions
	AsyncSocket
	debug
	File
	HTTP
	language
	lua_cjson
	luasocket
	Notify
	thread
	timer
)

