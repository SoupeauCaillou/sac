LOCAL_PATH := $(call my-dir)/../../../tremor

include $(CLEAR_VARS)

LOCAL_MODULE := libtremor

LOCAL_CFLAGS := -DANDROID_NDK \
				-I$(BASE_PATH) \
				-DLITTLE_ENDIAN=1 \
				-DBIG_ENDIAN=2 \
				-DBYTE_ORDER=LITTLE_ENDIAN

LOCAL_SRC_FILES = bitwiseARM.s \
	bitwise.c \
	codebook.c \
	dpenARM.s \
	dsp.c \
	floor0.c \
	floor1ARM.s \
	floor1.c \
	floor1LARM.s \
	floor_lookup.c \
	framing.c \
	info.c \
	mapping0.c \
	mdctARM.s \
	mdct.c \
	mdctLARM.s \
	misc.c \
	res012.c \
	vorbisfile.c

include $(BUILD_STATIC_LIBRARY)
