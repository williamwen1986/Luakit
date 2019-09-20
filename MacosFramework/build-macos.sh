#!/bin/bash

export TARGET=Luakit
. ../bin/build-macos.sh Luakit
checkError
export TARGET=Luakit-dylib
. ../bin/build-macos.sh Luakit
cp  -v build/$CONFIG/$TARGET.dylib  "$OUTPUT_DIR/Luakit.dylib"

checkError


