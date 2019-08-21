# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID -DHAVE_CONFIG_H -DUSE_OPENSSL -DUSE_SSLEAY -DHAVE_LIBZ -DHAVE_ZLIB_H")

set(outputDir $ENV{OUTPUT_DIR})

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/lib/*.cpp"
        "${PROJECT_SOURCE_DIR}/lib/*.c"
        "${PROJECT_SOURCE_DIR}/lib/vtls/*.c"
        "${PROJECT_SOURCE_DIR}/lib/vauth/*.c"
        )

add_library(
               # Sets the name of the library.
                curl

                # Sets the library as a shared library.
                STATIC

		${all_SRCS}
)

#set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../openssl-1.1.1c/outputs/${ANDROID_ABI})
#file(MAKE_DIRECTORY ${lib_build_DIR})
#add_subdirectory( "${PROJECT_SOURCE_DIR}/../openssl-1.1.1c" ${lib_build_DIR} )
#set_target_properties( ssl PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libssl.a)

link_directories(${outputDir}/${ANDROID_ABI})


add_library( ssl
             STATIC
             IMPORTED )
set_target_properties( # Specifies the target library.
                       ssl

                       # Specifies the parameter you want to define.
                       PROPERTIES IMPORTED_LOCATION

                       # Provides the path to the library you want to import.
                       ${outputDir}/${ANDROID_ABI}/libssl.a )
####target_link_libraries(base modp_b64 event ssl crypto)

add_library( crypto
             STATIC
             IMPORTED )
set_target_properties( # Specifies the target library.
                       crypto

                       # Specifies the parameter you want to define.
                       PROPERTIES IMPORTED_LOCATION

                       # Provides the path to the library you want to import.
                       ${outputDir}/${ANDROID_ABI}/libcrypto.a )



target_link_libraries( curl ssl crypto)
  # Specifies a path to native header files.
include_directories(
	$(PROJECT_SOURCE_DIR)/include
	$(PROJECT_SOURCE_DIR)/lib
	$(PROJECT_SOURCE_DIR)/../openssl-1.1.1c/include
	/usr/local/Cellar/node/11.5.0/include/node
)
