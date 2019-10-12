#!/bin/bash

#-------------------------------------------------------------------
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z "$ANDROID_API" ]
then
    export ANDROID_API=24
fi

DEFAULT_OUTPUT=../../libs/android"$ANDROID_API"-$CONFIG

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
export OS_ANDROID=1
make_abi() {
            ABI=$1
            echo
            echo "---------------------- ANDROID_ABI=$ABI ---------------------"
            rm -r obj CMakeFiles 2>/dev/null
            mkdir -p "$OUTPUT_DIR/$ABI"
            cmake .\
                -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
                -DANDROID_ABI=$ABI \
                -DANDROID_NATIVE_API_LEVEL=$ANDROID_API \
                -DANDROID_STL=c++_static \
                -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$OUTPUT_DIR/$ABI" \
                -DCMAKE_BUILD_TYPE=$CONFIG \
                -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$OUTPUT_DIR/$ABI" \
                -DOPENSSL_ROOT_DIR=../openssl-1.1.1c \
                -DOPENSSL_LIBRARIES="$OUTPUT_DIR/$ABI" \
                -DANDROID=1 \
                -DOS_ANDROID=1 \

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

export NDK_MODULE_PATH="$path"
export NDK_PROJECT_PATH="$path"
if [ -z "$NDK_ROOT" ]
then
	export NDK_ROOT=$ANDROID_NDK_HOME
fi
if [ -z "$ANDROID_NDK_HOME" ]
then
	export ANDROID_NDK_HOME=$NDK_ROOT
fi

rm -r obj CMakeFiles CMakeCache.txt 2>/dev/null
if [ .$BUILD = ."NDK_BUILD" ]; then
$ANDROID_NDK_HOME/ndk-build    NDK_APPLICATION_MK=Application.mk
cp -a obj/local/* "$OUTPUT_DIR"
rm -r obj 2>/dev/null
find "$OUTPUT_DIR" -name "objs*" -exec rm -rf {} \; 2>/dev/null
else

make_abi x86_64
make_abi x86
make_abi armeabi-v7a
make_abi arm64-v8a
fi

echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo
