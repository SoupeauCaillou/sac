noinst_LIBRARIES = libsac.a

AM_CXXFLAGS = $(GLFW_CFLAGS) \
	$(GLEW_CFLAGS) \
	$(OPENAL_CFLAGS) \
	$(LIBPNG_CFLAGS) \
	$(VORBIS_CFLAGS) \
	$(VORBISFILE_CFLAGS) \
	$(SQLITE_CFLAGS) \
	-DDATADIR=\"$(pkgdatadir)/\" \
	-DLOCALEDIR=\"$(localedir)/\"
	
libsac_a_SOURCES = api/AdAPI.h \
	api/AssetAPI.h \
	api/ExitAPI.h \
	api/LocalizeAPI.h \
	api/MusicAPI.h \
	api/NameInputAPI.h \
	api/SoundAPI.h \
	api/linux/AssetAPILinuxImpl.cpp \
	api/linux/ExitAPILinuxImpl.h \
	api/linux/MusicAPILinuxOpenALImpl.cpp \
	api/linux/NameInputAPILinuxImpl.h \
	api/linux/AssetAPILinuxImpl.h \
	api/linux/LocalizeAPILinuxImpl.cpp \
	api/linux/MusicAPILinuxOpenALImpl.h \
	api/linux/SoundAPILinuxOpenALImpl.cpp \
	api/linux/ExitAPILinuxImpl.cpp \
	api/linux/LocalizeAPILinuxImpl.h \
	api/linux/NameInputAPILinuxImpl.cpp \
	api/linux/SoundAPILinuxOpenALImpl.h \
	base/CircularBuffer.cpp \
	base/EntityManager.h \
	base/Log.h \
	base/PlacementHelper.cpp \
	base/TouchInputManager.cpp \
	base/Vector2.h \
	base/CircularBuffer.h \
	base/Interval.h \
	base/MathUtil.cpp \
	base/PlacementHelper.h \
	base/TimeUtil.cpp \
	base/TouchInputManager.h \
	base/EntityManager.cpp \
	base/Log.cpp \
	base/MathUtil.h \
	base/TimeUtil.h \
	base/Vector2.cpp \
	systems/ADSRSystem.cpp \
	systems/ParticuleSystem.cpp \
	systems/RenderingSystem_Render.cpp \
	systems/System.cpp \
	systems/ADSRSystem.h \
	systems/ParticuleSystem.h \
	systems/RenderingSystem_Texture.cpp \
	systems/System.h \
	systems/ButtonSystem.cpp \
	systems/MorphingSystem.cpp \
	systems/PhysicsSystem.cpp \
	systems/ScrollingSystem.cpp \
	systems/TextRenderingSystem.cpp \
	systems/ButtonSystem.h \
	systems/MorphingSystem.h \
	systems/PhysicsSystem.h \
	systems/ScrollingSystem.h \
	systems/TextRenderingSystem.h \
	systems/ContainerSystem.cpp \
	systems/MusicSystem.cpp \
	systems/RenderingSystem.cpp \
	systems/SoundSystem.cpp \
	systems/TransformationSystem.cpp \
	systems/ContainerSystem.h \
	systems/MusicSystem.h \
	systems/RenderingSystem.h \
	systems/SoundSystem.h \
	systems/TransformationSystem.h \
	util/ImageLoader.h \
	util/ImageLoader.cpp