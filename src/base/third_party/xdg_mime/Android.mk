LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  

LOCAL_MODULE := xdg_mime
LOCAL_SRC_FILES := \
				./xdgmime.c \
				./xdgmimealias.c \
				./xdgmimecache.c \
				./xdgmimeglob.c \
				./xdgmimeicon.c \
				./xdgmimeint.c \
				./xdgmimemagic.c \
				./xdgmimeparent.c \

LOCAL_C_INCLUDES += \
                    ../.. \
                    
LOCAL_LDFLAGS += -fPIC

include $(BUILD_STATIC_LIBRARY) 
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

include $(CLEAR_VARS)