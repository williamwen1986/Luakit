# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID -fvisibility=hidden")


add_library(      
                # Sets the name of the library.
                xml

                # Sets the library as a shared library.
                STATIC

                # Provides a relative path to your source file(s).
	src/c14n.c src/catalog.c src/chvalid.c src/debugXML.c src/dict.c src/DOCBparser.c
	src/encoding.c src/entities.c src/error.c src/globals.c src/hash.c src/HTMLparser.c
	src/HTMLtree.c src/legacy.c src/list.c src/nanoftp.c src/nanohttp.c src/parser.c
	src/parserInternals.c src/pattern.c src/relaxng.c src/SAX.c src/SAX2.c
	src/threads.c src/tree.c src/trionan.c src/triostr.c src/uri.c src/valid.c
	src/xinclude.c src/xlink.c src/xmlIO.c src/xmlmemory.c src/xmlmodule.c
	src/xmlreader.c src/xmlregexp.c src/xmlsave.c src/xmlschemas.c src/xmlschemastypes.c src/xmlstring.c
	src/xmlunicode.c src/xmlwriter.c src/xpath.c src/xpointer.c src/buf.c)


set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../libiconv-1.14/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../libiconv-1.14" ${lib_build_DIR} )
set_target_properties( iconv PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libiconv.a)

target_link_libraries(xml iconv)



# Specifies a path to native header files.
include_directories(
	${PROJECT_SOURCE_DIR}/src/include
)
