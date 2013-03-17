# - Find Vorbis libraries by Gautier :-).
# This module defines the following variables:
#  VORBIS_INCLUDE_DIRS - include directories for VORBIS
#  VORBIS_LIBRARIES - libraries to link against VORBIS
#  VORBIS_FOUND - true if VORBIS has been found and can be used

find_path(VORBIS_INCLUDE_DIR vorbis/vorbisfile.h
  HINTS
    ENV VORBISDIR
  PATH_SUFFIXES include
  PATHS 
  "$ENV{PROGRAMFILES}/../Program\ Files/Vorbis"
  ~/Library/Frameworks
  /Library/Frameworks
  /opt
  /usr/include/
)

find_library(VORBIS_LIBRARY 
	NAMES vorbis
  HINTS
    ENV VORBISDIR
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64 lib-msvc110
  PATHS
  "$ENV{PROGRAMFILES}/../Program\ Files/VPX"
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/lib/
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VORBIS
                                  REQUIRED_VARS VORBIS_INCLUDE_DIR VORBIS_LIBRARY)

mark_as_advanced(VORBIS_LIBRARY VORBIS_INCLUDE_DIR)
