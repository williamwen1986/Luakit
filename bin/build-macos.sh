#!/bin/bash

#-------------------------------------------------------------------
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $SDK_VERSION ]; then
	SDK_VERSION="10.14"
fi
PROJECT=$1

if [ -z $TARGET ]; then
    TARGET=$PROJECT
fi

DEFAULT_OUTPUT=../../libs/macos$SDK_VERSION-$CONFIG
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
pushd "$path" > /dev/null
dir=$(pwd)
path=$dir
popd > /dev/null


if [ -z "$OUTPUT_DIR" ]
then
     export OUTPUT_DIR="$DEFAULT_OUTPUT"
fi
mkdir -p $OUTPUT_DIR 2>/dev/null

pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
export OUTPUT_DIR="$dir"
popd > /dev/null

rm -rf DerivedData


xcodebuild -configuration $CONFIG -project $PROJECT.xcodeproj -target $TARGET -arch x86_64  -sdk macosx$SDK_VERSION -destination "platform=macOS,arch=x86_64" clean
checkError
echo "-----------------TARGET $TARGET"
xcodebuild -configuration $CONFIG -project $PROJECT.xcodeproj -target $TARGET -arch x86_64  -sdk macosx$SDK_VERSION -destination "platform=macOS,arch=x86_64" build
checkError

cp  -v build/$CONFIG/lib$TARGET.a  "$OUTPUT_DIR/lib$PROJECT.a"
echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo

