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

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/*.h"
        "${PROJECT_SOURCE_DIR}/*.hpp"
        "${PROJECT_SOURCE_DIR}/*.cpp"
        "${PROJECT_SOURCE_DIR}/*.c"
        )


add_library( luaFramework SHARED ${all_SRCS})

set(outputDir $ENV{OUTPUT_DIR})


##add_library( base STATIC IMPORTED )
##set_target_properties( base    PROPERTIES IMPORTED_LOCATION ${outputDir}/${ANDROID_ABI}/libbase.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../common/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../common" ${lib_build_DIR} )
###add_library( common STATIC IMPORTED )
set_target_properties( common PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libcommon.a)
###set_target_properties( common PROPERTIES IMPORTED_LOCATION ${outputDir}/libcommon.a)

#add_library( crypto STATIC IMPORTED )
#set_target_properties( crypto PROPERTIES IMPORTED_LOCATION ${outputDir}/${ANDROID_ABI}/libcrypto.a)

##add_library( curl STATIC IMPORTED )
##set_target_properties( curl PROPERTIES IMPORTED_LOCATION ${outputDir}/${ANDROID_ABI}/libcurl.a)

#add_library( event STATIC IMPORTED )
#set_target_properties( event PROPERTIES IMPORTED_LOCATION ${outputDir}/${ANDROID_ABI}/libevent.a)

#add_library( iconv STATIC IMPORTED )
#set_target_properties( iconv PROPERTIES IMPORTED_LOCATION ${outputDir}/${ANDROID_ABI}/libiconv.a)

#add_library( modp_b64 STATIC IMPORTED )
#set_target_properties( modp_b64 PROPERTIES IMPORTED_LOCATION ${outputDir}/${ANDROID_ABI}/libmodp_b64.a)

##add_library( ssl STATIC IMPORTED )
##set_target_properties( ssl PROPERTIES IMPORTED_LOCATION ${outputDir}/${ANDROID_ABI}/libssl.a)

##add_library( xml STATIC IMPORTED )
##set_target_properties( xml PROPERTIES IMPORTED_LOCATION ${outputDir}/${ANDROID_ABI}/libssl.a)


########find_library(mylib  $ENV{OUTPUT_DIR}/${ANDROID_ABI}/libxml.a)
link_directories(${outputDir}/${ANDROID_ABI})
target_link_libraries(luaFramework  common)
########link_libraries       (luaFramework common  crypto   ssl )

# Specifies a path to native header files.
include_directories(
                    $ENV{OUTPUT_DIR}/${ANDROID_ABI}
                    $(LOCALPROJECT_SOURCE_DIR_PATH)/.././
		    $(PROJECT_SOURCE_DIR)/../jni
                    $(PROJECT_SOURCE_DIR)/../common/
                    $(PROJECT_SOURCE_DIR)/../lua-5.1.5/lua
                    $(PROJECT_SOURCE_DIR)/../openssl/include/
                    $(PROJECT_SOURCE_DIR)/../curl/include
                    $(PROJECT_SOURCE_DIR)/../sqlite-amalgamation-3210000
                    $(PROJECT_SOURCE_DIR)/../extensions/HTTP
                    $(PROJECT_SOURCE_DIR)/../extensions/lua-cjson-master
                    $(PROJECT_SOURCE_DIR)/../extensions/thread
                    $(LOCAL_PROJECT_SOURCE_DIRPATH)/../extensions/timer
                    $(PROJECT_SOURCE_DIR)/../extensions/AsyncSocket
                    $(PROJECT_SOURCE_DIR)/../extensions/File
                    $(PROJECT_SOURCE_DIR)/../extensions/debug
                    $(PROJECT_SOURCE_DIR)/../extensions/luasocket
                    $(PROJECT_SOURCE_DIR)/../extensions/Notify
                    $(PROJECT_SOURCE_DIR)/../extensions/language
                    $(PROJECT_SOURCE_DIR)/../lua-5.1.5/lua/tools
)
