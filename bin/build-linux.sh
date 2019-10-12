#!/bin/bash

#-------------------------------------------------------------------
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi


DEFAULT_OUTPUT=../../libs/linux-$CONFIG


#--------------------------------------------------------------------

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

make_linux() {
            rm -r obj CMakeFiles 2>/dev/null
            cmake .\
                -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$OUTPUT_DIR" \
                -DCMAKE_BUILD_TYPE=$CONFIG \
                -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$OUTPUT_DIR" \
                -DOPENSSL_ROOT_DIR="../openssl-1.1.1c" \
                -DOPENSSL_LIBRARIES="$OUTPUT_DIR" \
                -DLINUX=1 \
                -DOS_LINUX=1 \

            checkError
            make -j4
            checkError
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
mkdir -p "$OUTPUT_DIR" 2>/dev/null

pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
export OUTPUT_DIR="$dir"
popd > /dev/null


rm -r obj CMakeFiles CMakeCache.txt 2>/dev/null

make_linux

echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo
