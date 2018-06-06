LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := modp_b64

LOCAL_CPP_EXTENSION := .cc 
LOCAL_SRC_FILES := \
					modp_b64.cc \

LOCAL_C_INCLUDES += \
					$(LOCAL_PATH) \
					$(LOCAL_PATH)/.. \

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_LDFLAGS += -fPIC

include $(BUILD_STATIC_LIBRARY)

