LOCAL_PATH:= $(call my-dir)
# We need to build this for both the device (as a shared library)
# and the host (as a static library for tools to use).

common_C_INCLUDES += \
	$(LOCAL_PATH)/src/include
# For the device
# =====================================================
include $(CLEAR_VARS)
LOCAL_MODULE:= libxml

LOCAL_SRC_FILES := src/c14n.c src/catalog.c src/chvalid.c src/debugXML.c src/dict.c src/DOCBparser.c \
	src/encoding.c src/entities.c src/error.c src/globals.c src/hash.c src/HTMLparser.c \
	src/HTMLtree.c src/legacy.c src/list.c src/nanoftp.c src/nanohttp.c src/parser.c \
	src/parserInternals.c src/pattern.c src/relaxng.c src/SAX.c src/SAX2.c \
	src/threads.c src/tree.c src/trionan.c src/triostr.c src/uri.c src/valid.c\
	src/xinclude.c src/xlink.c src/xmlIO.c src/xmlmemory.c src/xmlmodule.c \
	src/xmlreader.c src/xmlregexp.c src/xmlsave.c src/xmlschemas.c src/xmlschemastypes.c src/xmlstring.c \
	src/xmlunicode.c src/xmlwriter.c src/xpath.c src/xpointer.c src/buf.c

LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_C_INCLUDES += $(common_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDES += $(common_C_INCLUDES)

LOCAL_STATIC_LIBRARIES += iconv \

LOCAL_LDFLAGS += -fPIC

include $(BUILD_STATIC_LIBRARY)

$(call import-module, libiconv-1.14)