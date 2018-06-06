LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := common

LOCAL_CFLAGS := -DANDROID -DOS_ANDROID


MY_FILES_PATH  :=  $(LOCAL_PATH)/common \
				   $(LOCAL_PATH)/lua-5.1.5/lua \
				   $(LOCAL_PATH)/sqlite-amalgamation-3210000 \
				   $(LOCAL_PATH)/extensions \
				   $(LOCAL_PATH)/network \
				   $(LOCAL_PATH)/network/net \
				   

MY_FILTER_OUT_CONTAIN := %shell.c %.h %.hpp %.mm %.proto %test.cpp %mock_server.cpp %tests.cpp %_win.cc %_ios.cpp %.txt %html_utils.cpp %wwhttpapi.cpp %authhttpapi.cpp

# login_protocol_handler.cpp && mail_login_protocol_handler.cpp  ->  %login_protocol_handler.cpp 只合并这一个,task系列不太好用简化

My_All_Files := $(foreach src_path,$(MY_FILES_PATH), $(shell find "$(src_path)" -type f) ) 
My_All_Files := $(My_All_Files:$(LOCAL_PATH)/./%=$(LOCAL_PATH)%)
MY_SRC_LIST  := $(filter-out $(MY_FILTER_OUT_CONTAIN),$(My_All_Files)) 
MY_SRC_LIST  := $(MY_SRC_LIST:$(LOCAL_PATH)/%=%)
      
LOCAL_SRC_FILES += $(MY_SRC_LIST)

LOCAL_LDFLAGS += -fPIC

#$(warning "the value of LOCAL_SRC_FILES is $(LOCAL_SRC_FILES)")

LOCAL_C_INCLUDES += $(LOCAL_PATH)/./ \
                    $(LOCAL_PATH)/common/ \
                    $(LOCAL_PATH)/lua-5.1.5/lua \
                    $(LOCAL_PATH)/openssl/include/ \
                    $(LOCAL_PATH)/curl-7.43.0/include \
                    $(LOCAL_PATH)/sqlite-amalgamation-3210000 \
                    $(LOCAL_PATH)/extensions/HTTP \
                    $(LOCAL_PATH)/extensions/lua-cjson-master \
                    $(LOCAL_PATH)/extensions/thread \
                    $(LOCAL_PATH)/extensions/timer \
                    $(LOCAL_PATH)/extensions/AsyncSocket \
                    $(LOCAL_PATH)/extensions/File \
                    $(LOCAL_PATH)/extensions/Notify \
                    $(LOCAL_PATH)/jni \
                    $(LOCAL_PATH)/ \
                    $(LOCAL_PATH)/lua-5.1.5/lua/tools \


LOCAL_STATIC_LIBRARIES += curl \
                          base \
                          libxml \

include $(BUILD_STATIC_LIBRARY)

$(call import-module, curl-7.43.0)
$(call import-module, base)
$(call import-module, libxml)
