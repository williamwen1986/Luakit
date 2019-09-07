# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-multichar -DANDROID -DLIBDIR=c -DBUILDING_LIBICONV -DIN_LIBRARY")


add_library(      
                # Sets the name of the library.
                iconv

                # Sets the library as a shared library.
                STATIC

                # Provides a relative path to your source file(s).
		libcharset/lib/localcharset.c 
		lib/iconv.c 
		lib/relocatable.c
)

# Specifies a path to native header files.
include_directories(
				${PROJECT_SOURCE_DIR}/../
 				${PROJECT_SOURCE_DIR}/include
				${PROJECT_SOURCE_DIR}/libcharset
				${PROJECT_SOURCE_DIR}/lib
				${PROJECT_SOURCE_DIR}/libcharset/include
				${PROJECT_SOURCE_DIR}/srclib
)
