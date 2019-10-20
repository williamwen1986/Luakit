# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

file(GLOB ALL_SRCS
 
	"${PROJECT_SOURCE_DIR}/*.cpp"
)

if (${ANDROID})

else ()
    set (SRC lua_timer.cpp)
    list(FILTER ALL_SRCS EXCLUDE REGEX ".*com_common_luakit_LuaNotificationListener.cpp$")
    list(FILTER ALL_SRCS EXCLUDE REGEX ".*com_common_luakit_NativeHandleHolder.cpp$")
    list(FILTER ALL_SRCS EXCLUDE REGEX ".*com_common_luakit_NotificationHelper.cpp$")
    list(FILTER ALL_SRCS EXCLUDE REGEX ".*lua_notification_utility_android.cpp$")
    list(FILTER ALL_SRCS EXCLUDE REGEX ".*LogicModel.cpp")
    list(FILTER ALL_SRCS EXCLUDE REGEX ".*notification_listener_impl.cpp$")
endif ()

add_library(
                # Sets the name of the library.
                Notify

                # Sets the library as a shared library.
                STATIC

                ${ALL_SRCS}
)

target_link_libraries (Notify
                notification
)

# Specifies a path to native header files.
include_directories(
		    ${PROJECT_SOURCE_DIR}
            ${PROJECT_SOURCE_DIR}/..
		    ${PROJECT_SOURCE_DIR}/../../third-party/lua-5.1.5/src
		    ${PROJECT_SOURCE_DIR}/../../src
		    ${PROJECT_SOURCE_DIR}/../../include
		    ${PROJECT_SOURCE_DIR}/../../third-party
		    ${PROJECT_SOURCE_DIR}/../../src/common
		    ${PROJECT_SOURCE_DIR}/../../src/notification
		    ${PROJECT_SOURCE_DIR}/../../extensions/thread
		    ${PROJECT_SOURCE_DIR}/../../AndroidFramework/jni
 )
