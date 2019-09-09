#!/bin/bash

# Android
# -------
rm -rf src/base/CMakeFiles          src/base/outputs          src/base/CMakeCache.txt
rm -rf src/common/CMakeFiles        src/common/outputs        src/common/CMakeCache.txt
rm -rf src/curl/CMakeFiles          src/curl/outputs          src/curl/CMakeCache.txt
rm -rf src/libevent/CMakeFiles      src/libevent/outputs      src/libevent/CMakeCache.txt
rm -rf src/libiconv-1.14/CMakeFiles src/libiconv-1.14/outputs src/libiconv-1.14/CMakeCache.txt
rm -rf src/libxml/CMakeFiles        src/libxml/outputs        src/libxml/CMakeCache.txt
rm -rf src/modp_b64/CMakeFiles      src/modp_b64/outputs      src/modp_b64/CMakeCache.txt
rm -rf src/jni/CMakeFiles           src/jni/outputs           src/jni/CMakeCache.txt
#rm -rf src/openssl-1.1.1c/lib
rm -rf AndroidDemo/AsyncSocketTest/app/.cxx     AndroidDemo/AsyncSocketTest/app/.externalNativeBuild    AndroidDemo/AsyncSocketTest/app/build
rm -rf AndroidDemo/NotificationTest/app/.cxx    AndroidDemo/NotificationTest/app/.externalNativeBuild   AndroidDemo/NotificationTest/app/build
rm -rf AndroidDemo/OrmTest/app/.cxx             AndroidDemo/OrmTest/app/.externalNativeBuild            AndroidDemo/OrmTest/app/build
rm -rf AndroidDemo/ThreadTest/app/.cxx          AndroidDemo/ThreadTest/app/.externalNativeBuild         AndroidDemo/ThreadTest/app/build
rm -rf AndroidDemo/WeatherTest/app/.cxx         AndroidDemo/WeatherTest/app/.externalNativeBuild        AndroidDemo/WeatherTest/app/build
rm -rf src/jni/libluaFramework.so   src/jni/libs              src/jni/obj
rm -rf AndroidFrameWork/luakit/build AndroidFrameWork/lib_chromium/build
rm src/openssl-1.1.1c/Makefile

find "src" -name "CMakeFiles" -exec rm -rf {} \; 2>/dev/null
find "src" -name "outputs" -exec rm -rf {} \; 2>/dev/null
find "src" -name "CMakeCache.txt" -exec rm -rf {} \; 2>/dev/null
find "AndroidDemo" -name ".cxx" -exec rm -rf {} \; 2>/dev/null
find "AndroidDemo" -name ".externalNativeBuild" -exec rm -rf {} \; 2>/dev/null
find "AndroidDemo" -name "build" -exec rm -rf {} \; 2>/dev/null

# Macos
# -----
rm -rf src/modp_b64/build               src/modp_b64/DerivedData
find "DerivedData" -name "build" -exec rm -rf {} \; 2>/dev/null


# iOS
# ---
# TODO!
