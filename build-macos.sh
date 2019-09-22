#!/bin/bash

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $MACOS_SDK_VERSION ]; then
	export MACOS_SDK_VERSION="10.15"
fi

if [ -z "$OUTPUT_DIR" ]
then
export OUTPUT_DIR=libs/macos$MACOS_SDK_VERSION-$CONFIG
fi

macosdev=`xcode-select --print-path`
if [ ! -e "$macosdev/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$MACOS_SDK_VERSION.sdk" ]
then
    echo "$macosdev/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$MACOS_SDK_VERSION.sdk" not found
    echo "did you export the MACOS_SDK_VERSION environment variable ?"
    exit -1
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
./build-macos.sh
checkError
cd ../..

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
./build-macos.sh
checkError
cd ..

