LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
libponytech_la_SOURCES := $(call rwildcard, $(LOCAL_PATH)/,*.cpp)
LOCAL_SRC_FILES := $(libponytech_la_SOURCES)


LOCAL_C_INCLUDES += $(ponytech_INCLUDE) \
				$(LOCAL_PATH)/../ \
				$(LOCAL_PATH)/include/ \

LOCAL_CFLAGS := -Wall -g -std=c++14 -fexceptions $(OS_FLAG) $(ARCH_FLAG) 


LOCAL_MODULE := libponytechfrontends

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_LDLIBS += 

include $(BUILD_STATIC_LIBRARY)

