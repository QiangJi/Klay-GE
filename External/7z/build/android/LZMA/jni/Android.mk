LOCAL_PATH := $(call my-dir)
LZMA_PATH := $(LOCAL_PATH)
LZMA_SRC_PATH := $(LOCAL_PATH)/../../../../C/Util/LzmaLib

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LZMA_SRC_PATH)/../../
LOCAL_MODULE := LZMA
LOCAL_SRC_FILES := ../../Alloc.c ../../LzFind.c ../../LzmaDec.c ../../LzmaEnc.c ../../LzmaLib.c
LOCAL_PATH := $(LZMA_SRC_PATH)
LOCAL_CFLAGS := -DNO_ASM -D_7ZIP_ST
include $(BUILD_STATIC_LIBRARY)
