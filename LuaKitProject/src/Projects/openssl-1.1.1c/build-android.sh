#!/bin/sh
if [ -z "$CONFIG" ]
then
    export CONFIG=debug
fi

if [ -z "$ANDROID_API" ]
then
    export ANDROID_API=28
fi

DEFAULT_OUTPUT=../../../libs/android"$ANDROID_API"-$CONFIG
#--------------------------------------------------
path=$(dirname "$0")
OUTPUT_DIR=$1


# Checks exit value for error
#
checkError() {
    if [ $? -ne 0 ]
    then
        echo "Exiting due to errors (above)"
        exit -1
    fi
}



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

rm -r obj 2>/dev/null

if [ -f "lib/arm64-v8a/libcrypto.a" ] && [ -f "lib/armeabi-v7a/libcrypto.a" ] && [ -f "lib/x86/libcrypto.a" ] && [ -f "lib/x86_64/libcrypto.a" ] && [ -f "lib/arm64-v8a/libssl.a" ] && [ -f "lib/armeabi-v7a/libssl.a" ] && [ -f "lib/x86/libssl.a" ] && [ -f "lib/x86_64/libssl.a" ]; then
        echo "lib exists. Skipping openssl compilation"
else
        . ./buid-android-openssl.sh
fi

. ../../../bin/build-android.sh
checkError
