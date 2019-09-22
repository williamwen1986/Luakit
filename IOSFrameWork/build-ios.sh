#!/bin/bash

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $IOS_SDK_VERSION ]; then
	IOS_SDK_VERSION="13.0"
fi

if [ -z "$OUTPUT_DIR" ]
then
        export OUTPUT_DIR=../libs/ios$IOS_SDK_VERSION-$CONFIG
fi

iosdev=`xcode-select --print-path`
if [ ! -e "$iosdev/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS$IOS_SDK_VERSION.sdk" ]
then
    echo "$iosdev/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS$IOS_SDK_VERSION.sdk" not found
    echo "did you export the IOS_SDK_VERSION environment variable ?"
    exit -1
fi
if [ ! -e "$iosdev/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator$IOS_SDK_VERSION.sdk" ]
then
    echo "$iosdev/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator$IOS_SDK_VERSION.sdk"  not found
    echo "did you export the IOS_SDK_VERSION environment variable ?"
    exit -1
fi

. ../bin/build-ios.sh Luakit
checkError


