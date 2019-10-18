# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID -DUSE_HISTORICAL_STRERRO_R -DOS_ANDROID -DNVALGRIND -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti -fPIC -DUSE_HISTORICAL_STRERRO_R")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -llog -lz -lc")




set( lib_build_DIR ${PROJECT_SOURCE_DIR}/toluapp/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/toluapp" ${lib_build_DIR} )
set_target_properties( toluapp PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libtoluapp.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/lua-5.1.5/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/lua-5.1.5" ${lib_build_DIR} )
set_target_properties( lua PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/liblua.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/modp_b64/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/modp_b64" ${lib_build_DIR} )
set_target_properties( modp_b64 PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libmodp_b64.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/libxml/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/libxml" ${lib_build_DIR} )
set_target_properties( xml PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libxml.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/libevent/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/libevent" ${lib_build_DIR} )
set_target_properties( event PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libevent.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/icu/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/icu" ${lib_build_DIR} )
set_target_properties( icu PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libicu.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/dynamic_annotations/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/dynamic_annotations" ${lib_build_DIR} )
set_target_properties( dynamic_annotations PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libdynamic_annotations.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/dmg_fp/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/dmg_fp" ${lib_build_DIR} )
set_target_properties( dmg_fp PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libdmg_fp.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/curl/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/curl" ${lib_build_DIR} )
set_target_properties( curl PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libcurl.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/libiconv-1.14/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/libiconv-1.14" ${lib_build_DIR} )
set_target_properties( iconv PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libconv.a)



add_library(
                # Sets the name of the library.
                third_party

                # Sets the library as a shared library.
                STATIC

                # Provides a relative path to your source file(s).
                dummy.c
)

target_link_libraries( third_party 

                toluapp
                lua
                modp_b64
                xml
                event
                icu
                dynamic_annotations
                dmg_fp
                curl
                iconv
)

# Specifies a path to native header files.
include_directories(
                    ${PROJECT_SOURCE_DIR}
)
