#!/bin/bash

#-------------------------------------------------------------------
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $MACOS_SDK_VERSION ]; then
	export MACOS_SDK_VERSION="10.15"
fi
PROJECT=$1

if [ -z $TARGET ]; then
    TARGET=$PROJECT
fi

DEFAULT_OUTPUT=../libs/macos$MACOS_SDK_VERSION-$CONFIG
#-------------------------------------------------------------------

path=$(dirname "$0")

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
CURRENTDIR=$(pwd)
pushd "$path" > /dev/null
dir=$(pwd)
path="$dir"
popd > /dev/null


if [ -z "$OUTPUT_DIR" ]
then
     export OUTPUT_DIR="$DEFAULT_OUTPUT"
fi


macosdev=`xcode-select --print-path`
if [ ! -e "$macosdev/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$MACOS_SDK_VERSION.sdk" ]
then
    echo "$macosdev/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$MACOS_SDK_VERSION.sdk" not found
    echo "did you export the MACOS_SDK_VERSION environment variable ?"
    exit -1
fi


mkdir -p "$OUTPUT_DIR" 2>/dev/null

pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
export OUTPUT_DIR="$dir"
popd > /dev/null

rm -rf DerivedData

cd ../third-party/openssl-1.1.1c
. ./build-macos.sh
checkError
echo "cd $CURRENTDIR"
cd "$CURRENTDIR"

. ../bin/build-macos.sh luakit
checkError

echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo

