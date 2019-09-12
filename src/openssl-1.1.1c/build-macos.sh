#!/bin/sh

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi
LIB_ROOT=./libs

DEFAULT_OUTPUT=../../libs/macos-$CONFIG
#--------------------------------------------------
path=$(dirname "$0")
OUTPUT_DIR=$1



#
# Canonicalize relative paths to absolute paths
#
pushd "$path" > /dev/null
dir=$(pwd)
path=$dir
popd > /dev/null

if [ -z "$OUTPUT_DIR" ]
then
    mkdir -p $DEFAULT_OUTPUT 2>/dev/null
    OUTPUT_DIR="$DEFAULT_OUTPUT"
fi

pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
OUTPUT_DIR=$dir
popd > /dev/null

./Configure darwin64-x86_64-cc
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

    mkdir -p "${LIB_ROOT}/macos-$CONFIG/"

    rm ${LIB_ROOT}/macos-$CONFIG/libcrypto.dylib 2>/dev/null
    mv libcrypto.1.1.dylib ${LIB_ROOT}/macos-$CONFIG/
    rm ${LIB_ROOT}/macos-$CONFIG/libcrypto.a 2>/dev/null
    mv libcrypto.a ${LIB_ROOT}/macos-$CONFIG/
    rm ${LIB_ROOT}/macos-$CONFIG/libssl.a 2>/dev/null
    mv libssl.a ${LIB_ROOT}/macos-$CONFIG/
    rm ${LIB_ROOT}/macos-$CONFIG/libssl.dylib 2>/dev/null
    mv libssl.1.1.dylib ${LIB_ROOT}/macos-$CONFIG/

     # copy header
     mkdir -p "${LIB_ROOT}/macos-$CONFIG/include/openssl"
     cp -v -r "include/openssl" "${LIB_ROOT}/macos-$CONFIG/include/"


make clean

cp -a -v ${LIB_ROOT}/macos-$CONFIG/* "$OUTPUT_DIR"


echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo
