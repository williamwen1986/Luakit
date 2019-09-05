#!/bin/sh

# Android
# -------
rm -rf src/Projects/base/CMakeFiles          src/Projects/base/outputs          src/Projects/base/CMakeCache.txt
rm -rf src/Projects/common/CMakeFiles        src/Projects/common/outputs        src/Projects/common/CMakeCache.txt
rm -rf src/Projects/curl/CMakeFiles          src/Projects/curl/outputs          src/Projects/curl/CMakeCache.txt
rm -rf src/Projects/libevent/CMakeFiles      src/Projects/libevent/outputs      src/Projects/libevent/CMakeCache.txt
rm -rf src/Projects/libiconv-1.14/CMakeFiles src/Projects/libiconv-1.14/outputs src/Projects/libiconv-1.14/CMakeCache.txt
rm -rf src/Projects/libxml/CMakeFiles        src/Projects/libxml/outputs        src/Projects/libxml/CMakeCache.txt
rm -rf src/Projects/modp_b64/CMakeFiles      src/Projects/modp_b64/outputs      src/Projects/modp_b64/CMakeCache.txt
rm -rf src/Projects/jni/CMakeFiles           src/Projects/jni/outputs           src/Projects/jni/CMakeCache.txt
#rm -rf src/Projects/openssl-1.1.1c/lib
rm -rf AndroidDemo/AsyncSocketTest/app/.cxx     AndroidDemo/AsyncSocketTest/app/.externalNativeBuild    AndroidDemo/AsyncSocketTest/app/build
rm -rf AndroidDemo/NotificationTest/app/.cxx    AndroidDemo/NotificationTest/app/.externalNativeBuild   AndroidDemo/NotificationTest/app/build
rm -rf AndroidDemo/OrmTest/app/.cxx             AndroidDemo/OrmTest/app/.externalNativeBuild            AndroidDemo/OrmTest/app/build
rm -rf AndroidDemo/ThreadTest/app/.cxx          AndroidDemo/ThreadTest/app/.externalNativeBuild         AndroidDemo/ThreadTest/app/build
rm -rf AndroidDemo/WeatherTest/app/.cxx         AndroidDemo/WeatherTest/app/.externalNativeBuild        AndroidDemo/WeatherTest/app/build
rm -rf src/Projects/jni/libluaFramework.so   src/Projects/jni/libs              src/Projects/jni/obj
rm -rf AndroidFrameWork/luakit/build AndroidFrameWork/lib_chromium/build
rm src/Projects/openssl-1.1.1c/Makefile
