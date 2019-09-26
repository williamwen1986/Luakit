LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  

LOCAL_MODULE := dynamic_annotations
LOCAL_SRC_FILES := dynamic_annotations.c

LOCAL_C_INCLUDES += \
                    $(LOCAL_PATH)/../.. \
                    $(LOCAL_PATH)/../../.. \

LOCAL_CFLAGS := -DNVALGRIND

LOCAL_LDFLAGS += -fPIC

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_STATIC_LIBRARY) 