#!/bin/bash

. ../bin/build-macos.sh Luakit
checkError
. ../bin/build-macos.sh Luakit-dylib
checkError


