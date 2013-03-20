# - Find Vorbis libraries by Gautier :-).
# This module defines the following variables:
#  VORBISFILE_INCLUDE_DIRS - include directories for VORBISFILE
#  VORBISFILE_LIBRARIES - libraries to link against VORBISFILE
#  VORBISFILE_FOUND - true if VORBISFILE has been found and can be used


file(
	GLOB
	MIGHT_LOCATION
		"$ENV{PROGRAMFILES}/../Program\ Files/Vorbis"
		~/Library/Frameworks
		/Library/Frameworks
		/opt
)
	
find_path(VORBISFILE_INCLUDE_DIR vorbis/vorbisfile.h
	HINTS
		ENV VORBISFILEDIR
	PATH_SUFFIXES
		include/GL include
	PATHS 
		${MIGHT_LOCATION}
)

find_library(VORBISFILE_LIBRARY 
	NAMES
		vorbisfile 
	HINTS
		ENV VORBISFILEDIR
	PATH_SUFFIXES
		lib64 lib libs64 libs libs/Win32 libs/Win64
	PATHS 
		${MIGHT_LOCATION}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VORBISFILE
                                  REQUIRED_VARS VORBISFILE_INCLUDE_DIR VORBISFILE_LIBRARY)

mark_as_advanced(VORBISFILE_LIBRARY VORBISFILE_INCLUDE_DIR)
