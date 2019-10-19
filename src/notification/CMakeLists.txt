# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_HISTORICAL_STRERRO_R -DNVALGRIND -fvisibility=hidden")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti -fPIC -DUSE_HISTORICAL_STRERRO_R")

file(GLOB ALL_SRCS
        "${PROJECT_SOURCE_DIR}/*.cpp"
        "${PROJECT_SOURCE_DIR}/*.c"
        "${PROJECT_SOURCE_DIR}/*.cc"

)

if (${ANDROID})
    set (OS_SPECIFIC
    )
elseif (${LINUX})
                list(FILTER ALL_SRCS EXCLUDE REGEX ".*LogicModel.cpp$")
                list(FILTER ALL_SRCS EXCLUDE REGEX ".*notification_listener_impl.cpp$")
endif()

add_library(
                # Sets the name of the library.
                notification

                # Sets the library as a shared library.
                STATIC

                ${ALL_SRCS}
)



# Specifies a path to native header files.
include_directories(
                    ${PROJECT_SOURCE_DIR}/../
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/../../third-party
                    ${PROJECT_SOURCE_DIR}/../../third-party/lua-5.1.5/src
                    ${PROJECT_SOURCE_DIR}/../../third-party/openssl/include/
                    ${PROJECT_SOURCE_DIR}/../../third-party/curl/include
                    ${PROJECT_SOURCE_DIR}/../../third-party/sqlite-amalgamation-3290000
                    ###${PROJECT_SOURCE_DIR}/../../extensions/Notify
                    ${PROJECT_SOURCE_DIR}/../../AndroidFramework/jni
                    ${PROJECT_SOURCE_DIR}/../lua-tools
                    ${PROJECT_SOURCE_DIR}/../../include
                    ${PROJECT_SOURCE_DIR}/../common
)
