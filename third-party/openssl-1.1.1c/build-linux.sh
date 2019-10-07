#!/bin/bash

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi




DEFAULT_OUTPUT=../../libs/linux-$CONFIG
#--------------------------------------------------

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
./Configure linux-x86_64
if [ $? -ne 0 ]; then
    echo "Error executing: ./Configure darwin64-x86_64-cc $OPENSSL_CONFIGURE_OPTIONS"
    exit 1
fi
     make clean
     make -j4

     if [ $? -ne 0 ]; then
         echo "Error executing make for platform: linux-x86_64"
         exit 1
     fi

    cp libcrypto.so "$OUTPUT_DIR/"
    cp libcrypto.a "$OUTPUT_DIR/"
    cp libssl.a "$OUTPUT_DIR/"
    cp libssl.so "$OUTPUT_DIR/"

     # copy header
     mkdir -p "$OUTPUT_DIR/include/"
     cp -v -r "include/openssl" "$OUTPUT_DIR/include/"

    make clean

echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo
