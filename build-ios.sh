#!/bin/bash
#---------------------------------------------------
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

<<<<<<< HEAD
if [ -z $IOS_SDK_VERSION ]; then
	export IOS_SDK_VERSION="13.0"
=======
if [ -z $SDK_VERSION ]; then
	SDK_VERSION="12.4"
>>>>>>> Merge "build-macos" branch with William
fi

if [ -z "$OUTPUT_DIR" ]
then
<<<<<<< HEAD
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

=======
export OUTPUT_DIR=libs/ios$SDK_VERSION-$CONFIG
fi
#------------------------------------------------
>>>>>>> Merge "build-macos" branch with William
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
<<<<<<< HEAD

./clean.sh
rm -r "$OUTPUT_DIR"/* 2>/dev/null


cd third-party/openssl-1.1.1c
=======
./clean.sh
rm -r "$OUTPUT_DIR"/*


cd src/openssl-1.1.1c
./build-ios.sh
checkError
cd ../..


cd src/modp_b64
./build-ios.sh
checkError
cd ../..


cd src/libevent
>>>>>>> Merge "build-macos" branch with William
./build-ios.sh
checkError
cd ../..

<<<<<<< HEAD
=======
cd src/libxml
./build-ios.sh
checkError
cd ../..

cd src/curl
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
>>>>>>> Merge "build-macos" branch with William

cd IOSFrameWork
./build-ios.sh
checkError
cd ..

<<<<<<< HEAD
echo
echo "Your outputs are in $OUTPUT_DIR"
echo
=======
>>>>>>> Merge "build-macos" branch with William
