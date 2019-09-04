#!/bin/sh

# Checks exit value for error
#
checkError() {
    if [ $? -ne 0 ]
    then
        echo "Exiting due to errors (above)"
        exit -1
    fi
}


. ./build-android-openssl.sh
checkError

if [ .$BUILD = ."NDK_BUILD" ]; then
. ../../../bin/build-android.sh
checkError
fi

