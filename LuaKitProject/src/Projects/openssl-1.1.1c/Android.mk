LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := ssl
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libssl.a
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_LDFLAGS += -fPIC
#$(warning "----------------the value of LOCAL_SRC_FILES is:$(LOCAL_SRC_FILES)-------------------------")
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := crypto
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libcrypto.a
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_LDFLAGS += -fPIC
#$(warning "----------------the value of LOCAL_SRC_FILES is:$(LOCAL_SRC_FILES)-------------------------")
include $(PREBUILT_STATIC_LIBRARY)
