LOCAL_PATH := $(call my-dir)/../../

include $(CLEAR_VARS)

LOCAL_MODULE := sac

LOCAL_CFLAGS := -DANDROID_NDK \
				-I$(BASE_PATH)

LOCAL_CXXFLAGS := -DANDROID_NDK \
                -I$(LOCAL_PATH)/base \
				-I$(LOCAL_PATH)/systems

LOCAL_SRC_FILES := \
    base/MathUtil.cpp \
    base/Vector2.cpp \
    systems/TransformationSystem.cpp \
    systems/RenderingSystem.cpp \
    systems/PlayerSystem.cpp \
	systems/DebugRenderingManager.cpp

LOCAL_LDLIBS    += -lGLESv2 -llog -ldl

include $(BUILD_SHARED_LIBRARY)
