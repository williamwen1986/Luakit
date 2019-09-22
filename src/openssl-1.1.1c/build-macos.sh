#!/bin/sh

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $MACOS_SDK_VERSION ]; then
	MACOS_SDK_VERSION="10.15"
fi



LIB_ROOT=./

DEFAULT_OUTPUT=../../libs/macos$MACOS_MACOS_SDK_VERSION-$CONFIG
#--------------------------------------------------
path=$(dirname "$0")

#
# Canonicalize relative paths to absolute paths
#
if [ -z "$OUTPUT_DIR" ]
then
    mkdir -p "$DEFAULT_OUTPUT" 2>/dev/null
    OUTPUT_DIR="$DEFAULT_OUTPUT"
fi

pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
OUTPUT_DIR="$dir"
popd > /dev/null
#OPENSSL_CONFIGURE_OPTIONS="no-whirlpool no-ui no-engine -fPIC"

./Configure darwin64-x86_64-cc ${OPENSSL_CONFIGURE_OPTIONS}
if [ $? -ne 0 ]; then
    echo "Error executing:./Configure ${CONFIGURE_ARCH} -D__ANDROID_API__=$ANDROID_API ${OPENSSL_CONFIGURE_OPTIONS}"
    exit 1
fi

     make clean
     make -j4

     if [ $? -ne 0 ]; then
         echo "Error executing make for platform:${ANDROID_TARGET_PLATFORM}"
         exit 1
     fi
   LIB_ROOT = ./generation 
    mkdir -p "${LIB_ROOT}/macos-$CONFIG/"

    rm "${LIB_ROOT}/macos-$CONFIG/libcrypto.dylib" 2>/dev/null
    mv libcrypto.1.1.dylib "${LIB_ROOT}/macos-$CONFIG/"
    rm "${LIB_ROOT}/macos-$CONFIG/libcrypto.a" 2>/dev/null
    mv libcrypto.a "${LIB_ROOT}/macos-$CONFIG/"
    rm "${LIB_ROOT}/macos-$CONFIG/libssl.a" 2>/dev/null
    mv libssl.a "${LIB_ROOT}/macos-$CONFIG/"
    rm "${LIB_ROOT}/macos-$CONFIG/libssl.dylib" 2>/dev/null
    mv libssl.1.1.dylib "${LIB_ROOT}/macos-$CONFIG/"

     # copy header
     mkdir -p "${LIB_ROOT}/macos-$CONFIG/include-x86_64/openssl-x86_64"
     cp -v -r "include/openssl" "${LIB_ROOT}/macos-$CONFIG/include-x86_64/"

    make clean

cp -v -r "${LIB_ROOT}/macos-$CONFIG/" "$OUTPUT_DIR"
echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo
