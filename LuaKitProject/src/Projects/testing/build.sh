# !/bin/sh 

DEPLOY_PATH=`pwd`/../../protocol/lib

MODULE_NAME=gtest

BUILDDIR="`pwd`/../../build/gtest.build"

IOS_SDK_VERSION=6.1
XCODE_ROOT=/Applications/Xcode.app/Contents/Developer
LIPO=$XCODE_ROOT/Platforms/iPhoneOS.platform/Developer/usr/bin/lipo
export IPHONEOS_DEPLOYMENT_TARGET="4.3"

abort()
{
    echo
    echo "Aborted: $@"
    exit 1
}

doneSection()
{
    echo
    echo "    ================================================================="
    echo "    Done"
    echo
}

xcodebuild -project $MODULE_NAME.xcodeproj -sdk iphoneos${IOS_SDK_VERSION} -scheme $MODULE_NAME -configuration Release

xcodebuild -project $MODULE_NAME.xcodeproj -sdk iphonesimulator${IOS_SDK_VERSION} -scheme $MODULE_NAME -configuration Release


LIBNAME=gtest

ARMLIB=$BUILDDIR/Release-iphoneos/lib$LIBNAME.a

SIMLIB=$BUILDDIR/Release-iphonesimulator/lib$LIBNAME.a

$LIPO -create "$ARMLIB" "$SIMLIB"  -o "$DEPLOY_PATH/lib$LIBNAME.a" || abort "Lipo $1 failed"

#COPY HEADER

HEADER_PATH=$DEPLOY_PATH/../include/$MODULE_NAME/

[ -d $HEADER_PATH ] || mkdir $HEADER_PATH

cp -R ./gtest/include/* $HEADER_PATH
