#!/bin/bash

#-------------------------------------------------------------------
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi
DEFAULT_OUTPUT=../../libs/android"$ANDROID_API"-$CONFIG
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
export OUTPUT_DIR=$dir
popd > /dev/null

xcodebuild -configuration $CONFIG -project $1_macos.xcodeproj
checkError


echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo

