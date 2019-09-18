#!/bin/bash
#---------------------------------------------------
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $SDK_VERSION ]; then
	SDK_VERSION="12.4"
fi

if [ -z "$OUTPUT_DIR" ]
then
export OUTPUT_DIR=libs/ios$SDK_VERSION-$CONFIG
fi
#------------------------------------------------
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


cd src/openssl-1.1.1c
./build-ios.sh
checkError
cd ../..


cd src/modp_b64
./build-ios.sh
checkError
cd ../..


cd src/libevent
./build-ios.sh
checkError
cd ../..

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

cd IOSFrameWork
./build-ios.sh
checkError
cd ..

