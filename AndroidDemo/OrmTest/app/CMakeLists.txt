cmake_minimum_required(VERSION 3.4.1)
project(luakit)
set( LUAKIT_ROOT ${PROJECT_SOURCE_DIR}/../../.. )
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -llog -lz -lc")
file(GLOB jni_SRCS "${LUAKIT_ROOT}/AndroidFramework/jni/*.cpp")

add_library(luakitApp SHARED LuakitExtensions.cpp ${jni_SRCS} )

set( lib_build_DIR ${LUAKIT_ROOT}/AndroidFramework/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${LUAKIT_ROOT}/AndroidFramework"  ${lib_build_DIR})
set_target_properties( luaFramework PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libluaFramework.a)


target_link_libraries( luakitApp luaFramework )

include_directories(
                    ${LUAKIT_ROOT}/AndroidFramework
                    ${LUAKIT_ROOT}/AndroidFramework/jni
                    ${LUAKIT_ROOT}/src
                    ${LUAKIT_ROOT}/src/common
                    ${LUAKIT_ROOT}/include
                    ${LUAKIT_ROOT}/third-party
                    ${LUAKIT_ROOT}/third-party/lua-5.1.5/src
                    ${LUAKIT_ROOT}/src/lua-tools
)

