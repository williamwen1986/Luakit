# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_HISTORICAL_STRERRO_R -DNVALGRIND -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti -fPIC -DUSE_HISTORICAL_STRERRO_R")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -llog -lz -lc")


add_library(
                # Sets the name of the library.
                dmg_fp

                # Sets the library as a shared library.
                STATIC

                # Provides a relative path to your source file(s).
                dtoa.cc
                g_fmt.cc
)

# Specifies a path to native header files.
include_directories(
                ${PROJECT_SOURCE_DIR}
 )
