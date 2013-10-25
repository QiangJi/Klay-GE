LOCAL_PATH := $(call my-dir)
MESHMLLIB_PATH := $(LOCAL_PATH)
MESHMLLIB_SRC_PATH := $(LOCAL_PATH)/../../../src

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(MESHMLLIB_SRC_PATH)/../../External/boost \
		$(MESHMLLIB_SRC_PATH)/../../KFL/include \
		$(MESHMLLIB_SRC_PATH)/../include \
		
LOCAL_MODULE := MeshMLLib
LOCAL_PATH := $(MESHMLLIB_SRC_PATH)
CPP_FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(CPP_FILE_LIST:$(LOCAL_PATH)/%=%)
		
LOCAL_CFLAGS := -DMESHMLLIB_SOURCE

LOCAL_LDLIBS := -llog

include $(BUILD_STATIC_LIBRARY)
