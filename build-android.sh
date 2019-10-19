#!/bin/bash
#------------------------------
#
# To build LuaKit for Android :
#
#    $ export ANDROID_NDK_HOME=....
#    $ cd luakit/LuaKitProject
#    $ export CONFIG=Debug
#    $ export ANDROID_API=24
#    $ ./build-android.sh
#
# Note : CONFIG=Debug or Release (with an uppercase character) for a CMAKE build.
#      : CONFIG=debug or release (with a lowercase character) for a NDK_BUILD build
#
#------------------------------ 
if [ -z "$BUILD" ]
then
#export BUILD=NDK_BUILD
 export BUILD=CMAKE
 fi

if [ -z "$CONFIG" ]
then
 export CONFIG=Debug
#export CONFIG=debug
fi

if [ -z "$ANDROID_API" ]
then
  export ANDROID_API=24
fi

DEFAULT_OUTPUT=libs/android"$ANDROID_API"-$CONFIG
#------------------------------



if [ ! -d "$ANDROID_HOME/build-tools/" ]
then
    	echo "$0: directory '$ANDROID_HOME/build-tools/' not found."
	    echo "Have you set correctly \$ANDROID_HOME environment variable ?"
	    exit -1
fi

if [ -z "$ANDROID_NDK_HOME" ]
then
	    export ANDROID_NDK_HOME=$ANDROID_NDK
fi

if [ ! -d "$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/" ]
then
    	echo "$0: directory '$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/' not found."
	    echo "Have you set \$ANDROID_NDK_HOME environment variable ?"
	    exit -1
fi

if [ ! -e "$ANDROID_NDK_HOME/platforms/android-$ANDROID_API" ]
then
    echo "$ANDROID_NDK_HOME/platforms/android-$ANDROID_API" not found
    echo "did you export the ANDROID_API environment variable ?"
    exit -1
fi


# Checks exit value for error
#
checkError() {
    if [ $? -ne 0 ]
    then
        echo "Exiting due to errors (above)"
        exit -1
    fi
}


if [ -z "$1" ]
then
        export BUILD=$BUILD
else
        export BUILD=$1
fi

if [ -z "$BUILD" ]
then
    export build=NDK_BUILD
fi

#
# Canonicalize relative paths to absolute paths
#
pushd "$path" > /dev/null
dir=$(pwd)
path="$dir"
popd > /dev/null


if [ -z  "$OUTPUT_DIR" ]
then
     export OUTPUT_DIR="$DEFAULT_OUTPUT"
fi

rm -rf  "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR" 2>/dev/null
pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
export OUTPUT_DIR="$dir"
popd > /dev/null

cd third-party/openssl-1.1.1c
./build-android.sh
checkError
cd ../..

cd AndroidFramework
./build-android.sh 
checkError
cd ..
