#!/bin/sh
export PATH=$ANDROID_NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH

if [ -z "$ANDROID_API" ]
then
    export ANDROID_API=28
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

rm -r lib 2>/dev/null

./Configure android-x86_64 -D__ANDROID_API__=$ANDROID_API
checkError
make clean
checkError
make
checkError
mkdir -p lib/x86_64
cp libcrypto.a lib/x86_64
cp libssl.a lib/x86_64

./Configure android-x86 -D__ANDROID_API__=$ANDROID_API
checkError
make clean
checkError
make
checkError
mkdir -p lib/x86
cp libcrypto.a lib/x86
cp libssl.a lib/x86

./Configure android-arm64 -D__ANDROID_API__=$ANDROID_API
checkError
make clean
checkError
make
checkError
mkdir -p lib/arm64-v8a
cp libcrypto.a lib/arm64-v8a
cp libssl.a lib/arm64-v8a

./Configure android-arm -D__ANDROID_API__=$ANDROID_API
checkError
make clean
checkError
make
checkError
mkdir -p lib/armeabi-v7a
cp libcrypto.a lib/armeabi-v7a
cp libssl.a lib/armeabi-v7a

make clean
checkError


cp -a lib/* "$OUTPUT_DIR"

echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo
