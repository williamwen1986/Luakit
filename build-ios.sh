#!/bin/bash
#---------------------------------------------------
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $IOS_SDK_VERSION ]; then
	export IOS_SDK_VERSION="12.2"
fi

if [ -z "$OUTPUT_DIR" ]
then
    export OUTPUT_DIR=libs/ios$IOS_SDK_VERSION-$CONFIG
fi
#------------------------------------------------
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

mkdir -p "$OUTPUT_DIR" 2>/dev/null
pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
export OUTPUT_DIR=$dir
popd > /dev/null

checkError() {
    if [ $? -ne 0 ]
    then
        echo "Exiting due to errors (above)"
        exit -1
    fi
}

./clean.sh
rm -r "$OUTPUT_DIR"/*


cd third-party/openssl-1.1.1c
./build-ios.sh
checkError
cd ../..

cd third-party/dynamic_annotations
./build-ios.sh
checkError
cd ../..

cd third-party/dmg_fp
./build-ios.sh
checkError
cd ../..

cd third-party/toluapp
./build-ios.sh
checkError
cd ../..

cd third-party/modp_b64
./build-ios.sh
checkError
cd ../..

cd third-party/lua-5.3.5
./build-ios.sh
checkError
cd ../..

cd third-party/curl
./build-ios.sh
checkError
cd ../..

cd third-party/libxml
./build-ios.sh
checkError
cd ../..

cd third-party/libevent
./build-ios.sh
checkError
cd ../..

cd src/common
./build-ios.sh
checkError
cd ../..

cd src/base
./build-ios.sh
checkError
cd ../..

cd IOSFrameWork
./build-ios.sh
checkError
cd ..

