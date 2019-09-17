#!/bin/sh

if [ -z "$ANDROID_API" ]
then
    export ANDROID_API=24
fi
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi


DEFAULT_OUTPUT=../../libs/android"$ANDROID_API"-$CONFIG
#--------------------------------------------------
path=$(dirname "$0")
#
# Canonicalize relative paths to absolute paths
#
pushd "$path" > /dev/null
dir=$(pwd)
path=$dir
popd > /dev/null

if [ -z "$OUTPUT_DIR" ]
then
    mkdir -p $DEFAULT_OUTPUT 2>/dev/null
    OUTPUT_DIR="$DEFAULT_OUTPUT"
fi

pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
OUTPUT_DIR="$dir"
popd > /dev/null

rm -r obj 2>/dev/null

#if [ -f "lib/arm64-v8a/libcrypto.a" ] && [ -f "lib/armeabi-v7a/libcrypto.a" ] && [ -f "lib/x86/libcrypto.a" ] && [ -f "lib/x86_64/libcrypto.a" ] && [ -f "lib/arm64-v8a/libssl.a" ] && [ -f "lib/armeabi-v7a/libssl.a" ] && [ -f "lib/x86/libssl.a" ] && [ -f "lib/x86_64/libssl.a" ]; then
#        echo "lib exists. Skipping openssl compilation"
#else
#       
#fi

if [ -z "$NDK_ROOT" ]
then
        export NDK_ROOT=$ANDROID_NDK_HOME
fi
if [ -z "$ANDROID_NDK_HOME" ]
then
        export ANDROID_NDK_HOME=$NDK_ROOT
fi

if [ ! ${ANDROID_NDK_HOME} ]; then
     echo "ANDROID_NDK_HOME environment variable not set, set and rerun"
     exit 1
 fi


ANDROID_LIB_ROOT=./libs
ANDROID_TOOLCHAIN_DIR=/tmp/android-toolchain

HOST_INFO=`uname -a`
case ${HOST_INFO} in
     Darwin*)
         TOOLCHAIN_SYSTEM=darwin-x86_64
         ;;
     Linux*)
         if [[ "${HOST_INFO}" == *i686* ]]
         then
             TOOLCHAIN_SYSTEM=linux-x86
         else
             TOOLCHAIN_SYSTEM=linux-x86_64
         fi
         ;;
     *)
         echo "Toolchain unknown for host system : ${HOST_INFO}"
         exit 1
         ;;
esac

#./Configure dist
export PATH=${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/${TOOLCHAIN_SYSTEM}/bin:$PATH

for ANDROID_TARGET_PLATFORM in x86 x86_64 arm64-v8a armeabi-v7a
 do
     echo "Building for libcrypto.a and libssl.a for ${ANDROID_TARGET_PLATFORM}"
     case "${ANDROID_TARGET_PLATFORM}" in
         armeabi)
             TOOLCHAIN_ARCH=arm
             TOOLCHAIN_PREFIX=arm-linux-androideabi
             CONFIGURE_ARCH=android
             PLATFORM_OUTPUT_DIR=armeabi
             OPENSSL_CONFIGURE_OPTIONS="no-whirlpool no-ui no-engine -fPIC"
             ;;
         armeabi-v7a)
             TOOLCHAIN_ARCH=arm
             TOOLCHAIN_PREFIX=arm-linux-androideabi
             CONFIGURE_ARCH=android-arm 
             PLATFORM_OUTPUT_DIR=armeabi-v7a
             OPENSSL_CONFIGURE_OPTIONS="no-whirlpool no-ui no-engine -fPIC"
             ;;
         x86)
             TOOLCHAIN_ARCH=x86
             TOOLCHAIN_PREFIX=i686-linux-android
             CONFIGURE_ARCH=android-x86
             PLATFORM_OUTPUT_DIR=x86
             OPENSSL_CONFIGURE_OPTIONS="no-whirlpool no-ui no-engine -fPIC"
             ;;
         x86_64)
             TOOLCHAIN_ARCH=x86_64
             TOOLCHAIN_PREFIX=x86_64-linux-android
             CONFIGURE_ARCH=android-x86_64
             PLATFORM_OUTPUT_DIR=x86_64
             OPENSSL_CONFIGURE_OPTIONS="no-whirlpool no-ui no-engine -fPIC"
             ;;
         arm64-v8a)
             TOOLCHAIN_ARCH=arm64
             TOOLCHAIN_PREFIX=aarch64-linux-android
             CONFIGURE_ARCH=android-arm64
             PLATFORM_OUTPUT_DIR=arm64-v8a
             OPENSSL_CONFIGURE_OPTIONS="no-whirlpool no-ui no-engine -fPIC"
             ;;
         *)
             echo "Unsupported build platform:${ANDROID_TARGET_PLATFORM}"
             exit 1
     esac

    rm -rf "${ANDROID_TOOLCHAIN_DIR}"
    mkdir -p "${ANDROID_LIB_ROOT}/android$ANDROID_API-$CONFIG/${PLATFORM_OUTPUT_DIR}"

    echo "./Configure ${CONFIGURE_ARCH} -D__ANDROID_API__=$ANDROID_API ${OPENSSL_CONFIGURE_OPTIONS}\n"
    ./Configure ${CONFIGURE_ARCH} -D__ANDROID_API__=$ANDROID_API  ${OPENSSL_CONFIGURE_OPTIONS}

     if [ $? -ne 0 ]; then
         echo "Error executing:./Configure ${CONFIGURE_ARCH} -D__ANDROID_API__=$ANDROID_API ${OPENSSL_CONFIGURE_OPTIONS}"
         exit 1
     fi

     make clean
     make -j4

     if [ $? -ne 0 ]; then
         echo "Error executing make for platform:${ANDROID_TARGET_PLATFORM}"
         exit 1
     fi

     cp -v libcrypto.a "${ANDROID_LIB_ROOT}/android$ANDROID_API-$CONFIG/${PLATFORM_OUTPUT_DIR}"
     cp -v libssl.a "${ANDROID_LIB_ROOT}/android$ANDROID_API-$CONFIG/${PLATFORM_OUTPUT_DIR}"
     cp -v libcrypto.so.1.1 "${ANDROID_LIB_ROOT}/android$ANDROID_API-$CONFIG/${PLATFORM_OUTPUT_DIR}/libcrypto.1.1.so"
     cp -v libssl.so.1.1 "${ANDROID_LIB_ROOT}/android$ANDROID_API-$CONFIG/${PLATFORM_OUTPUT_DIR}/libssl.1.1.so"

     # copy header
     mkdir -p "${ANDROID_LIB_ROOT}/android$ANDROID_API-$CONFIG/${PLATFORM_OUTPUT_DIR}/include/openssl"
     cp -r -v "include/openssl" "${ANDROID_LIB_ROOT}/android$ANDROID_API-$CONFIG/${PLATFORM_OUTPUT_DIR}/include/"
 done 


make clean

cp -a  ${ANDROID_LIB_ROOT}/android"$ANDROID_API"-$CONFIG/* "$OUTPUT_DIR"

if [ .$BUILD = ."NDK_BUILD" ]; then
. ../../bin/build-android.sh
checkError
fi


echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo
