#!/bin/bash

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

<<<<<<< HEAD
if [ -z $MACOS_SDK_VERSION ]; then
	export MACOS_SDK_VERSION="10.15"
=======
if [ -z $SDK_VERSION ]; then
	SDK_VERSION="10.14"
>>>>>>> Merge "build-macos" branch with William
fi

if [ -z "$OUTPUT_DIR" ]
then
<<<<<<< HEAD
export OUTPUT_DIR=libs/macos$MACOS_SDK_VERSION-$CONFIG
fi

macosdev=`xcode-select --print-path`
if [ ! -e "$macosdev/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$MACOS_SDK_VERSION.sdk" ]
then
    echo "$macosdev/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$MACOS_SDK_VERSION.sdk" not found
    echo "did you export the MACOS_SDK_VERSION environment variable ?"
    exit -1
=======
export OUTPUT_DIR=libs/macos$SDK_VERSION-$CONFIG
>>>>>>> Merge "build-macos" branch with William
fi

mkdir -p $OUTPUT_DIR 2>/dev/null
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
./build-macos.sh
checkError
cd ../..


cd src/modp_b64
./build-macos.sh
checkError
cd ../..


cd src/libevent
>>>>>>> Merge "build-macos" branch with William
./build-macos.sh
checkError
cd ../..

<<<<<<< HEAD


cd IOSFrameWork
=======
cd src/libxml
./build-macos.sh
checkError
cd ../..


cd src/curl
./build-macos.sh
checkError
cd ../..


cd src/common
./build-macos.sh
checkError
cd ../..

cd src/base
./build-macos.sh
checkError
cd ../..


cd MacosFramework
>>>>>>> Merge "build-macos" branch with William
./build-macos.sh
checkError
cd ..

<<<<<<< HEAD
echo
echo "Your outputs are in $OUTPUT_DIR"
echo
=======
>>>>>>> Merge "build-macos" branch with William
