
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := event

LOCAL_CFLAGS := \
				-DHAVE_SYS_TIME_H \
				-DANDROID \
				-Wno-multichar \
				-DHAVE_CONFIG_H \
				-DDYNAMIC_ANNOTATIONS_ENABLED=1 \
				-DOS_ANDROID \

LOCAL_SRC_FILES := \
				buffer.c \
				epoll.c \
				epoll_sub.c \
				evbuffer.c \
				evdns.c \
				event.c \
				event_tagging.c \
				evrpc.c \
				evutil.c \
				http.c \
				log.c \
				poll.c \
				select.c \
				sample/event-test.c \
				sample/signal-test.c \
				sample/time-test.c \
				signal.c \
				strlcpy.c \
				test/bench.c \
				test/regress.c \
				test/regress.gen.c \
				test/regress_dns.c \
				test/regress_http.c \
				test/regress_rpc.c \
				test/test-eof.c \
				test/test-init.c \
				test/test-time.c \
				test/test-weof.c \


LOCAL_C_INCLUDES += \
				$(LOCAL_PATH) \
				$(LOCAL_PATH)/android \
				$(LOCAL_PATH)/compat \
				$(LOCAL_PATH)/compat/sys \
				$(LOCAL_PATH)/freebsd \
				$(LOCAL_PATH)/linux \
				$(LOCAL_PATH)/mac \
				$(LOCAL_PATH)/sample \
				$(LOCAL_PATH)/solaris \
				$(LOCAL_PATH)/test \

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_LDFLAGS += -fPIC

include $(BUILD_STATIC_LIBRARY)

