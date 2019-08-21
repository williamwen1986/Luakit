 #!/bin/sh
#------------------------------ 
#export BUILD=NDK_BUILD
 export BUILD=CMAKE

 export CONFIG=debug
#export CONFIG=release

export ANDROID_API=28
DEFAULT_OUTPUT=libs/android"$ANDROID_API"-$CONFIG
#------------------------------


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

cd src/Projects/openssl-1.1.1c
./build-android-openssl.sh
checkError
cp -v -a lib/* $OUTPUT_DIR
cd ../../..

cd src/Projects/jni
./build-android.sh
checkError
cd ../../..
