#!/bin/bash

#-------------------------------------------------------------------
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $SDK_VERSION ]; then
	SDK_VERSION="12.4"
fi

PROJECT=$1

if [ -z $TARGET ]; then
    TARGET=$PROJECT
fi

DEFAULT_OUTPUT=../../libs/ios$SDK_VERSION-$CONFIG
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
mkdir -p "$OUTPUT_DIR" 2>/dev/null

pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
export OUTPUT_DIR="$dir"
popd > /dev/null

rm -rf DerivedData

LIB_ROOT=./libs
rm -r $LIB_ROOT
mkdir -p $LIB_ROOT

buildIOS()
{
	ARCH=$1

    xcodebuild clean -configuration $CONFIG -project $PROJECT.xcodeproj -target $TARGET -arch $1 -destination $2 -sdk $3$SDK_VERSION 
    checkError
    xcodebuild build -configuration $CONFIG -project $PROJECT.xcodeproj -target $TARGET -arch $1 -destination $2 -sdk $3$SDK_VERSION 
    checkError
    cp -v build/$CONFIG-$3/lib$PROJECT.a $LIB_ROOT/$PROJECT-$1.a
}

    buildIOS "arm64"  "platform=iOS,arch=armv64"            "iphoneos"
    buildIOS "x86_64" "platform=iOS Simulator,arch=x86_64"  "iphonesimulator"

echo "Building iOS libraries"
lipo \
	"${LIB_ROOT}/$PROJECT-arm64.a" \
	"${LIB_ROOT}/$PROJECT-x86_64.a" \
	-create -output "$OUTPUT_DIR/lib$1.a"

echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo

