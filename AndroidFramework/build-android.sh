#!/bin/bash

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z "$ANDROID_API" ]
then
    export ANDROID_API=24
fi

if [ -z "$OUTPUT_DIR" ]
then
        export OUTPUT_DIR="../libs/android"$ANDROID_API"-$CONFIG"
        mkdir -p "$OUTPUT_DIR" 2>/dev/null
fi

. ../bin/build-android.sh
checkError
