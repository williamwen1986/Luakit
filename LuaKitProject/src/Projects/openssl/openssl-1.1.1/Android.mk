LOCAL_PATH := $(call my-dir)

# include $(CLEAR_VARS)
# LOCAL_MODULE    := openssl
# LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/lib/libssl.so 
# include $(PREBUILT_SHARED_LIBRARY)

# include $(CLEAR_VARS)
# LOCAL_MODULE    := opencrypto
# LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/lib/libcrypto.so
# include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := ssl
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libssl.a
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_LDFLAGS += -fPIC
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := crypto
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libcrypto.a
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_LDFLAGS += -fPIC
include $(PREBUILT_STATIC_LIBRARY)