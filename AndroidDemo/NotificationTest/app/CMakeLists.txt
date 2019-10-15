cmake_minimum_required(VERSION 3.4.1)
project(luakit)
#cmake_policy(SET CMP0079 NEW)
set( LUAKIT_ROOT ${PROJECT_SOURCE_DIR}/../../.. )
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -llog -lz -lc")

add_library(
                # Sets the name of the library.
                NotificationTest

                # Sets the library as a shared library.
                SHARED

                # Provides a relative path to your source file(s).
                LuakitExtensions.cpp
)

####link_directories(${LUAKIT_ROOT}/libs/android${ANDROID_NATIVE_API_LEVEL}-${CMAKE_BUILD_TYPE})

set( lib_build_DIR ${LUAKIT_ROOT}/AndroidFramework/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${LUAKIT_ROOT}/AndroidFramework"  ${lib_build_DIR})
set_target_properties( luaFramework PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libluaFramework.a)


target_link_libraries( NotificationTest
                 luaFramework
  
)
