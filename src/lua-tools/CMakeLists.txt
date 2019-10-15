
cmake_minimum_required(VERSION 3.4.1)
project(luakit)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID -DUSE_HISTORICAL_STRERRO_R -DOS_ANDROID -DNVALGRIND -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti -fPIC -DUSE_HISTORICAL_STRERRO_R")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -llog -lz -lc")

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/*.cpp"
        )

list(FILTER all_SRCS EXCLUDE REGEX ".*lua_timer.cpp$")
list(FILTER all_SRCS EXCLUDE REGEX ".*shell.c$")



add_library( lua_tools 
        STATIC 
        ${all_SRCS}
)


# Specifies a path to native header files.
include_directories(
                    ${PROJECT_SOURCE_DIR}
)

include_directories(
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/..
                    ${PROJECT_SOURCE_DIR}/../../third-party/lua-5.1.5/src
                    ${PROJECT_SOURCE_DIR}/../../third-party/sqlite-amalgamation-3290000
                    ${PROJECT_SOURCE_DIR}/../../third-party/toluapp
                    ${PROJECT_SOURCE_DIR}/../../third-party/toluapp/include
                    ${PROJECT_SOURCE_DIR}/../base
                    ${PROJECT_SOURCE_DIR}/../../include
)
