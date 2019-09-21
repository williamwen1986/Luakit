LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := crypto
LOCAL_SRC_FILES := $(LOCAL_PATH)/lib/$(TARGET_ARCH_ABI)/libcrypto.a
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include
#$(warning "----------------the value of LOCAL_SRC_FILES is: $(LOCAL_SRC_FILES) :-------------------------")
LOCAL_LDFLAGS += -fPIC
CFLAGS=-D__ANDROID_API__=$API

include $(PREBUILT_STATIC_LIBRARY)