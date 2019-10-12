#!/bin/bash


if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $ANDROID_API ]; then
	export ANDROID_API="24"
fi

if [ -z "$OUTPUT_DIR" ]
then
        export OUTPUT_DIR=../libs/android$ANDROID_API-$CONFIG
fi

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

. ../bin/build-android.sh
checkError
