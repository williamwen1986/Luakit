#!/bin/bash

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $SDK_VERSION ]; then
	SDK_VERSION="10.14"
fi

if [ -z "$OUTPUT_DIR" ]
then
export OUTPUT_DIR=libs/macos$SDK_VERSION-$CONFIG
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

