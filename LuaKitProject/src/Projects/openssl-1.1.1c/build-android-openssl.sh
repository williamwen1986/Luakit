#!/bin/sh

if [ -z "$ANDROID_API" ]
then
    export ANDROID_API=24
fi
if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi


DEFAULT_OUTPUT=../../../libs/android"$ANDROID_API"-$CONFIG
#--------------------------------------------------
path=$(dirname "$0")
OUTPUT_DIR=$1



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
OUTPUT_DIR=$dir
popd > /dev/null

rm -r obj 2>/dev/null

#if [ -f "lib/arm64-v8a/libcrypto.a" ] && [ -f "lib/armeabi-v7a/libcrypto.a" ] && [ -f "lib/x86/libcrypto.a" ] && [ -f "lib/x86_64/libcrypto.a" ] && [ -f "lib/arm64-v8a/libssl.a" ] && [ -f "lib/armeabi-v7a/libssl.a" ] && [ -f "lib/x86/libssl.a" ] && [ -f "lib/x86_64/libssl.a" ]; then
#        echo "lib exists. Skipping openssl compilation"
#else
#       
#fi


if [ ! ${ANDROID_NDK_HOME} ]; then
     echo "ANDROID_NDK_HOME environment variable not set, set and rerun"
     exit 1
 fi


ANDROID_LIB_ROOT=./lib
ANDROID_TOOLCHAIN_DIR=/tmp/android-toolchain
#OPENSSL_CONFIGURE_OPTIONS="no-whirlpool no-ui -fPIC \
#        no-pic no-krb5 no-idea no-camellia \
#        no-seed no-bf no-cast no-rc2 no-rc4 no-rc5 no-md2 \
#        no-md4 no-ripemd no-rsa no-ecdh no-sock no-ssl2 no-ssl3 \
#        no-dsa no-dh no-ec no-ecdsa no-tls1 no-pbe no-pkcs \
#        no-tlsext no-pem no-rfc3779 no-whirlpool no-ui no-srp \
#        no-ssltrace no-tlsext no-mdc2 no-ecdh no-engine \
#        no-tls2 no-srtp -fPIC"
#OPENSSL_CONFIGURE_OPTIONS="-fPIC"


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
             OPENSSL_CONFIGURE_OPTIONS="no-whirlpool no-engine -fPIC"
             ;;
         *)
             echo "Unsupported build platform:${ANDROID_TARGET_PLATFORM}"
             exit 1
     esac

     rm -rf ${ANDROID_TOOLCHAIN_DIR}
     mkdir -p "${ANDROID_LIB_ROOT}/android"$ANDROID_API"-$CONFIG/${PLATFORM_OUTPUT_DIR}"
     #python ${ANDROID_NDK_HOME}/build/tools/make_standalone_toolchain.py \
     #       --arch ${TOOLCHAIN_ARCH} \
     #       --api ${ANDROID_API} \
     #       --install-dir ${ANDROID_TOOLCHAIN_DIR}

     if [ $? -ne 0 ]; then
         echo "Error executing make_standalone_toolchain.py for ${TOOLCHAIN_ARCH}"
         exit 1
     fi

     export PATH=${ANDROID_NDK}/toolchains/llvm/prebuilt/${TOOLCHAIN_SYSTEM}/bin:$PATH
     #export PATH=$ANDROID_NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH

     #export CROSS_SYSROOT=${ANDROID_TOOLCHAIN_DIR}/sysroot

#     RANLIB=${TOOLCHAIN_PREFIX}-ranlib \
#           AR=${TOOLCHAIN_PREFIX}-ar \
#           CC=${TOOLCHAIN_PREFIX}-gcc \
#           ./Configure "${CONFIGURE_ARCH}" \
#           -D__ANDROID_API__=${ANDROID_API} \
#           "${OPENSSL_CONFIGURE_OPTIONS}"

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

     mv libcrypto.a ${ANDROID_LIB_ROOT}/android"$ANDROID_API"-$CONFIG/${PLATFORM_OUTPUT_DIR}
     mv libssl.a ${ANDROID_LIB_ROOT}/android"$ANDROID_API"-$CONFIG/${PLATFORM_OUTPUT_DIR}

     # copy header
     mkdir -p "${ANDROID_LIB_ROOT}/android"$ANDROID_API"-$CONFIG/${PLATFORM_OUTPUT_DIR}/include/openssl"
     cp -r "include/openssl" "${ANDROID_LIB_ROOT}/android"$ANDROID_API"-$CONFIG/${PLATFORM_OUTPUT_DIR}/include/"
 done 


#if [  -f "lib/x86_64/libcrypto.a" ]  && [ -f "lib/x86_64/libssl.a" ]; then
#    echo "lib/x86_64/libssl.a exists. Skipping"
#else
#    ./Configure android-x86_64 -D__ANDROID_API__=$ANDROID_API
#fi

#if [  -f "lib/x86/libcrypto.a" ] && [ -f "lib/x86/libssl.a" ]; then
#    echo "lib/x86/libssl.a exists. Skipping"
#else
#   ./Configure android-x86 -D__ANDROID_API__=$ANDROID_API 
#fi

#if [  -f "lib/arm64-v8a/libcrypto.a" ] && [ -f "lib/arm64-v8a/libssl.a" ]; then
#    echo "lib/arm64-v8a/libssl.a exists. Skipping"
#else
#    ./Configure android-arm64 -D__ANDROID_API__=$ANDROID_API 
#i

#if [  -f "lib/armeabi-v7a/libcrypto.a" ] && [ -f "lib/armeabi-v7a/libssl.a" ]; then
#    echo "lib/armeabi-v7a/libssl.a exists. Skipping"
#else
#    ./Configure android-arm -D__ANDROID_API__=$ANDROID_API no-asm no-shared no-unit-test
#fi

make clean

cp -a -v ${ANDROID_LIB_ROOT}/android"$ANDROID_API"-$CONFIG/* "$OUTPUT_DIR"

echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo
