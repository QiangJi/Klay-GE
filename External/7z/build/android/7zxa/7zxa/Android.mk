LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := 7zxa
LOCAL_SRC_FILES := ../obj/local/$(TARGET_ARCH_ABI)/lib7zxa.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../../
LOCAL_EXPORT_LDLIBS := -llog

include $(PREBUILT_STATIC_LIBRARY)
