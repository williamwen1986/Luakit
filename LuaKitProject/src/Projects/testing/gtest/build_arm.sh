#!/bin/sh

# SEE: http://www.smallsharptools.com/downloads/libical/

PATH="`xcode-select -print-path`/usr/bin:/usr/bin:/bin"
OUTPUTDIR=../gtest-build

# clear output
find $OUTPUTDIR -name \*.a -exec rm {} \;

sh build.sh armv7
sh build.sh armv7s

LIPO="xcrun -sdk iphoneos lipo"

$LIPO -arch armv7 $OUTPUTDIR/armv7/gtest.a -arch armv7s $OUTPUTDIR/armv7s/gtest.a -output $OUTPUTDIR/gtest.a -create

$LIPO -info $OUTPUTDIR/gtest.a
