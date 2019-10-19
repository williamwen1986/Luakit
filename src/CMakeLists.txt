# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

 
set( lib_build_DIR ${PROJECT_SOURCE_DIR}/base/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/base" ${lib_build_DIR} )
set_target_properties( base PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libbase.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/common/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/common" ${lib_build_DIR} )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libcommon.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/notification/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/notification" ${lib_build_DIR} )
set_target_properties( notification PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libnotification.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/network/lua-tools/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/lua-tools" ${lib_build_DIR} )
set_target_properties( lua_tools PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/liblua_tools.a)



add_library(
                # Sets the name of the library.
                luakit_engine

                # Sets the library as a shared library.
                STATIC

                dummy.c
)

target_link_libraries( luakit_engine
    base
    common
    notification
    lua_tools
)

