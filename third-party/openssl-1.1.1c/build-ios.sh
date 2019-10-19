#!/bin/bash

if [ -z "$CONFIG" ]
then
    export CONFIG=Debug
fi

if [ -z $IOS_SDK_VERSION ]; then
	IOS_SDK_VERSION="13.1"
fi


DEFAULT_OUTPUT=../../libs/ios$IOS_SDK_VERSION-$CONFIG
#--------------------------------------------------
#
# Canonicalize relative paths to absolute paths
#
if [ -z "$OUTPUT_DIR" ]
then
    mkdir -p "$DEFAULT_OUTPUT" 2>/dev/null
    OUTPUT_DIR="$DEFAULT_OUTPUT"
fi

pushd "$OUTPUT_DIR" > /dev/null
dir=$(pwd)
OUTPUT_DIR="$dir"
popd > /dev/null
DEVELOPER=`xcode-select -print-path`

LIB_ROOT=./generation
rm -r $LIB_ROOT
buildIOS()
{
	ARCH=$1
    	OPENSSL_CONFIGURE_OPTIONS="no-whirlpool no-ui no-engine -fPIC"
	if [[ "$ARCH" == "i386" || "$ARCH" == "x86_64" ]]; then
		PLATFORM="iPhoneSimulator"
	else
		PLATFORM="iPhoneOS"
	fi
  
	export $PLATFORM
   	export CROSS_TOP="$DEVELOPER/Platforms/$PLATFORM.platform/Developer"
	export CROSS_SDK="$PLATFORM$IOS_SDK_VERSION.sdk"
	export BUILD_TOOLS="$DEVELOPER"
	export CC="$BUILD_TOOLS/usr/bin/gcc -arch $ARCH"
    	mkdir -p "$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/$ARCH"

	echo "Building $OPENSSL_VERSION for $PLATFORM  $IOS_SDK_VERSION $ARCH"

	if [[ "$ARCH" == "x86_64" ]]; then
		./Configure darwin64-x86_64-cc   $OPENSSL_CONFIGURE_OPTIONS
	else
		./Configure iphoneos-cross  $OPENSSL_CONFIGURE_OPTIONS
	fi
	if [ $? -ne 0 ]; then
		echo "Error executing:./Configure $ARCH  $OPENSSL_CONFIGURE_OPTIONS"
		exit 1
	fi
	sed -ie "s!^CFLAG=!CFLAG=-isysroot $CROSS_TOP/SDKs/$CROSS_SDK -miphoneos-version-min=$IOS_SDK_VERSION !" "Makefile"

	make clean
	make -j4 
	if [ $? -ne 0 ]; then
		echo "Error executing make for platform:$ARCH"
		exit 1
	fi
	mkdir -p "$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/$ARCH"
	cp -v libcrypto.a "$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/$ARCH"
	cp -v libssl.a "$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/$ARCH"
	cp -v libcrypto.so.1.1 "$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/$ARCH/libcrypto.1.1.so"
	cp -v libssl.so.1.1 "$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/$ARCH/libssl.1.1.so"

	# copy header
	mkdir -p "$OUTPUT_DIR/include-$ARCH/openssl"
	cp -r -v "include/openssl" "$OUTPUT_DIR/include-$ARCH/"
}


    buildIOS "arm64"
    buildIOS "x86_64"


echo "Building iOS libraries"
lipo \
	"$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/arm64/libcrypto.a" \
	"$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/x86_64/libcrypto.a" \
	-create -output "$OUTPUT_DIR/libcrypto.a"

lipo \
	"$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/arm64/libssl.a" \
	"$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/x86_64/libssl.a" \
	-create -output "$OUTPUT_DIR/libssl.a"

make clean
    
#cp -r "$LIB_ROOT/ios$IOS_SDK_VERSION-$CONFIG/" "$OUTPUT_DIR"
echo
echo "Your ouputs are in " "$OUTPUT_DIR"
echo
