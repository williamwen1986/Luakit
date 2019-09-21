#!/bin/sh

# SEE: http://www.smallsharptools.com/downloads/libical/

ARCH=$1

PATH="`xcode-select -print-path`/usr/bin:/usr/bin:/bin"

# set the prefix
PREFIX=${HOME}/Library/gtest
OUTPUTDIR=../gtest-build
# Select the desired iPhone SDK
export SDKVER="6.1"
export DEVROOT=`xcode-select --print-path`
export SDKROOT=$DEVROOT/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${SDKVER}.sdk
export IOSROOT=$DEVROOT/Platforms/iPhoneOS.platform

# finding ld
# find $DEVROOT -type f -name ld|grep -i iphone

# Set up relevant environment variables 
export CPPFLAGS="-arch $ARCH -stdlib=libc++ -I$SDKROOT/usr/include -I$IOSROOT/Developer/usr/llvm-gcc-4.2/lib/gcc/arm-apple-darwin10/4.2.1/include"
export CFLAGS="$CPPFLAGS -pipe -no-cpp-precomp -isysroot $SDKROOT -I$PWD -I$PWD/include"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-L$SDKROOT/usr/lib/ -arch $ARCH"

export CLANG=$DEVROOT/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
export CC=$CLANG
export CXX=$CLANG
export LD=$IOSROOT/Developer/usr/bin/ld
export AR=$IOSROOT/Developer/usr/bin/ar 
export AS=$IOSROOT/Developer/usr/bin/as 
#export LIBTOOL=$IOSROOT/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/libtool 
#export STRIP=$IOSROOT/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/strip 
#export RANLIB=$IOSROOT/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/ranlib

HOST=arm-apple-darwin10

find . -name \*.a -exec rm {} \;

#make clean
#./configure --prefix=$PREFIX --disable-dependency-tracking --host $HOST CXX=$CXX CC=$CC LD=$LD AR=$AR AS=$AS
#LIBTOOL=$LIBTOOL STRIP=$STRIP RANLIB=$RANLIB
#make -j4
$CLANG $CXXFLAGS $LDFLAGS -c $PWD/src/gtest-all.cc
#$CLANG $LDFLAGS $OUTPUTDIR/gtest.a 
# copy the files to the arch folder

mkdir -p $OUTPUTDIR
mkdir -p $OUTPUTDIR/$ARCH

cp `find . -name \*.a` $OUTPUTDIR/$ARCH/

xcrun -sdk iphoneos lipo -info $OUTPUTDIR/$ARCH/*.a

echo $ARCH DONE

echo "See $OUTPUTDIR"
