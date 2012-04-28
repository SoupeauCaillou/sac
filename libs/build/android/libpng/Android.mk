LOCAL_PATH := $(call my-dir)/../../../libpng
include $(CLEAR_VARS)

LOCAL_CFLAGS := -DANDROID_NDK \
				-I$(LOCAL_PATH)

LOCAL_MODULE    := libpng
LOCAL_SRC_FILES :=\
	jni/png.c \
	jni/pngerror.c \
	jni/pngget.c \
	jni/pngmem.c \
	jni/pngpread.c \
	jni/pngread.c \
	jni/pngrio.c \
	jni/pngrtran.c \
	jni/pngrutil.c \
	jni/pngset.c \
	jni/pngtrans.c \
	jni/pngwio.c \
	jni/pngwrite.c \
	jni/pngwtran.c \
	jni/pngwutil.c 
	
LOCAL_LDLIBS := -lz

include $(BUILD_STATIC_LIBRARY)
