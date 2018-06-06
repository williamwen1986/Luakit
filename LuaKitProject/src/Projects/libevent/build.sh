
DEPLOY_PATH=`pwd`/../../protocol/lib

MODULE_NAME=event

OTHER_BUILD_FLAGS = ""

IOS_SDK_VERSION=6.1

XCODE_ROOT=/Applications/Xcode.app/Contents/Developer

LIPO=$XCODE_ROOT/Platforms/iPhoneOS.platform/Developer/usr/bin/lipo

export IPHONEOS_DEPLOYMENT_TARGET="4.3"

BUILD=`pwd`/../../build/event.build

ARM_MAKEFILE=`pwd`/Makefile.arm

SIM_MAKEFILE=`pwd`/Makefile.sim


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
build()
{
	HOST=arm-apple-darwin10
	ARCH=$1
	TARGET=iPhoneOS
	if [ "$1" == "sim" ]
	then 
		TARGET=iPhoneSimulator
		ARCH=i386
		HOST=i386-apple-darwin
	fi

	export CC="${XCODE_ROOT}/Platforms/$TARGET.platform/Developer/usr/bin/gcc"

	export LD="${XCODE_ROOT}/Platforms/$TARGET.platform/Developer/usr/bin/ld"

	export CXX="${XCODE_ROOT}/Platforms/$TARGET.platform/Developer/usr/bin/g++"

	export CFLAGS="-pipe -Os -gdwarf-2 -arch $ARCH -isysroot ${XCODE_ROOT}/Platforms/${TARGET}.platform/Developer/SDKs/${TARGET}${IOS_SDK_VERSION}.sdk"

	export LDFLAGS="-arch $ARCH -isysroot ${XCODE_ROOT}/Platforms/$TARGET.platform/Developer/SDKs/$TARGET${IOS_SDK_VERSION}.sdk"

	export CPPFLAGS="-D__IPHONE_OS_VERSION_MIN_REQUIRED=${IPHONEOS_DEPLOYMENT_TARGET%%.*}0000"

	
	mkdir -p ${BUILD}/$1

	make clean

	echo "./configure --disable-shared --enable-static ${OTHER_BUILD_FLAGS} --host=$HOST --prefix=${BUILD}/$1"
	./configure --disable-shared --enable-static ${OTHER_BUILD_FLAGS} --host=$HOST --prefix=${BUILD}/$1 || abort "configure failed"

	doneSection

	make  && make install

	doneSection
}

build armv7

build armv7s

build sim

LIBNAME=event

ARMV7LIB=$BUILD/armv7/lib/lib$LIBNAME.a

ARMV7SLIB=$BUILD/armv7s/lib/lib$LIBNAME.a

SIMLIB=$BUILD/sim/lib/lib$LIBNAME.a

$LIPO -create "$ARMV7LIB" "$ARMV7SLIB" "$SIMLIB"  -o "$DEPLOY_PATH/lib$LIBNAME.a" || abort "Lipo $1 failed"

doneSection

#copy header

cp -R $BUILD/armv7/include/* $DEPLOY_PATH/../include/$MODULE_NAME/


