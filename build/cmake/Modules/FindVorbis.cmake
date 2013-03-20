# - Find Vorbis libraries by Gautier :-).
# This module defines the following variables:
#  VORBIS_INCLUDE_DIRS - include directories for VORBIS
#  VORBIS_LIBRARIES - libraries to link against VORBIS
#  VORBIS_FOUND - true if VORBIS has been found and can be used


file(
	GLOB
	MIGHT_LOCATION
		"$ENV{PROGRAMFILES}/../Program\ Files/Vorbis"
		~/Library/Frameworks
		/Library/Frameworks
		/opt
        /usr
)
	
find_path(VORBIS_INCLUDE_DIR vorbis/vorbisfile.h
  HINTS
    ENV VORBISDIR
  PATH_SUFFIXES
    include
  PATHS 
    ${MIGHT_LOCATION}
)

find_library(VORBIS_LIBRARY 
  NAMES
    vorbis
  HINTS
    ENV VORBISDIR
  PATH_SUFFIXES
    lib64 lib libs64 libs libs/Win32 libs/Win64 so a
  PATHS 
    ${MIGHT_LOCATION}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VORBIS
                                  REQUIRED_VARS VORBIS_INCLUDE_DIR VORBIS_LIBRARY)

mark_as_advanced(VORBIS_LIBRARY VORBIS_INCLUDE_DIR)
