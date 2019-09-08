#!/bin/bash

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z "$OUTPUT_DIR" ]
then
     export OUTPUT_DIR="../../../../libs/macos-$CONFIG"
fi

. ../../../../bin/build-macos.sh dynamic_annotations
checkError

