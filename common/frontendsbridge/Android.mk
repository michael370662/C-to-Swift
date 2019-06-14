LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
libponytech_la_SOURCES := $(call rwildcard, $(LOCAL_PATH)/,*.c)
LOCAL_SRC_FILES := $(libponytech_la_SOURCES)


LOCAL_C_INCLUDES += \
	$(strongswan_PATH)/src \
	$(strongswan_PATH)/src/libipsec \
	$(strongswan_PATH)/src/libcharon \
	$(strongswan_PATH)/src/libstrongswan \
	$(ponytech_INCLUDE) 


LOCAL_CFLAGS := $(strongswan_CFLAGS)

LOCAL_MODULE := libfrontendsbridge

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_LDLIBS += 

LOCAL_SHARED_LIBRARIES = libstrongswan libipsec libcharon 

include $(BUILD_STATIC_LIBRARY)

