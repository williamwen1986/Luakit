#!/bin/bash

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z "$OUTPUT_DIR" ]
then
     export OUTPUT_DIR="../../../../libs/ios-$CONFIG"
fi

. ../../../../bin/build-ios.sh dynamic_annotations
checkError

