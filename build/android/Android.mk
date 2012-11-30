LOCAL_PATH := $(call my-dir)/../../

include $(CLEAR_VARS)

LOCAL_MODULE := sac

LOCAL_CFLAGS := -DANDROID_NDK -DENABLE_LOG \
				-I$(BASE_PATH)

LOCAL_CXXFLAGS := -DANDROID_NDK -DENABLE_LOG  \
				-I$(LOCAL_PATH)/base \
				-I$(LOCAL_PATH)/systems \
				-I$(LOCAL_PATH)/libs \
				-I$(LOCAL_PATH)/libs/libpng/jni \
                -ffast-math -O3 -funroll-loops

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
	base/Game.cpp \
	base/MathUtil.cpp \
	base/Vector2.cpp \
	base/Color.cpp \
	base/CircularBuffer.cpp \
	base/EntityManager.cpp \
	base/Profiler.cpp \
	base/TouchInputManager.cpp \
	base/Log.cpp \
	base/PlacementHelper.cpp \
	base/TimeUtil.cpp \
	steering/BasicSteeringBehavior.cpp \
	systems/ADSRSystem.cpp \
	systems/AnimationSystem.cpp \
	systems/AutoDestroySystem.cpp \
	systems/AutonomousAgentSystem.cpp \
	systems/ButtonSystem.cpp \
	systems/ContainerSystem.cpp \
	systems/MorphingSystem.cpp \
	systems/MusicSystem.cpp \
	systems/ParticuleSystem.cpp \
	systems/PhysicsSystem.cpp \
	systems/RenderingSystem.cpp \
	systems/RenderingSystem_Texture.cpp \
	systems/RenderingSystem_Render.cpp \
	systems/ScrollingSystem.cpp \
	systems/SoundSystem.cpp \
	systems/System.cpp \
	systems/TransformationSystem.cpp \
	systems/TextRenderingSystem.cpp \
	api/android/AdAPIAndroidImpl.cpp \
	api/android/AssetAPIAndroidImpl.cpp \
	api/android/MusicAPIAndroidImpl.cpp \
	api/android/SoundAPIAndroidImpl.cpp \
	api/android/LocalizeAPIAndroidImpl.cpp \
	api/android/NameInputAPIAndroidImpl.cpp \
	api/android/ExitAPIAndroidImpl.cpp \
	api/android/SuccessAPIAndroidImpl.cpp \
	api/android/VibrateAPIAndroidImpl.cpp \
	api/android/CommunicationAPIAndroidImpl.cpp \
	util/ImageLoader.cpp \
	util/IntersectionUtil.cpp \
	util/Serializer.cpp



#LOCAL_LDLIBS	+= -lGLESv2 -llog

include $(BUILD_STATIC_LIBRARY)
