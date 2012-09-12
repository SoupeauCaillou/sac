LOCAL_PATH := $(call my-dir)/../../../jsoncpp-src-0.6.0-rc2/src/lib_json
include $(CLEAR_VARS)

LOCAL_MODULE := libjsoncpp

LOCAL_CXXFLAGS := -DANDROID_NDK \
				-I$(LOCAL_PATH)/../../include/

LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES = json_reader.cpp \
	json_writer.cpp \
	json_value.cpp

include $(BUILD_STATIC_LIBRARY)
