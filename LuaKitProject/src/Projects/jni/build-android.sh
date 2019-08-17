#!/bin/sh

#-------------------------------------------------------------------
if [ -z "$CONFIG" ]
then
    export CONFIG=debug
fi

if [ -z "$ANDROID_API" ]
then
    export ANDROID_API=28
fi

DEFAULT_OUTPUT=../../../libs/android"$ANDROID_API"-$CONFIG

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

make_abi() {
            ABI=$1
            echo
            echo "---------------------- ANDROID_ABI=$ABI ---------------------"
            rm -r obj CMakeFiles 2>/dev/null
            mkdir -p $OUTPUT_DIR/$ABI
            cmake .\
                -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
                -DANDROID_ABI=$ABI \
                -DANDROID_NATIVE_API_LEVEL=$ANDROID_API \
                -DANDROID_STL=c++_static \
                -DLIBRARY_OUTPUT_DIRECTORY=$CONFIG \
                -DCMAKE_BUILD_TYPE=$CONFIG \
                -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=$OUTPUT_DIR/$ABI
            checkError
            make
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
mkdir -p $OUTPUT_DIR 2>/dev/null

pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
export OUTPUT_DIR=$dir
popd > /dev/null

export NDK_MODULE_PATH="$path"
export NDK_PROJECT_PATH="$path"
echo $OUTPUT_DIR

rm -r obj CMakeFiles 2>/dev/null
if [ .$BUILD = ."NDK_BUILD" ]; then
$ANDROID_NDK_HOME/ndk-build    NDK_APPLICATION_MK=Application.mk
pwd
echo $OUTPUT_DIR
cp -a obj/local/* "$OUTPUT_DIR"
#rm -r obj 2>/dev/null
#find "$OUTPUT_DIR" -name "objs*" -exec rm -rf {} \; 2>/dev/null
else
make_abi x86_64
make_abi x86
make_abi armeabi-v7a
make_abi arm64-v8a
fi

echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo
