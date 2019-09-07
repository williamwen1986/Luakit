# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library(
                # Sets the name of the library.
                modp_b64

                # Sets the library as a shared library.
                STATIC

                # Provides a relative path to your source file(s).
                modp_b64.cc
)

# Specifies a path to native header files.
include_directories(
                "${PROJECT_SOURCE_DIR}"
)
