# - Find Vorbis libraries by Gautier :-).
# This module defines the following variables:
#  VORBISFILE_INCLUDE_DIRS - include directories for VORBISFILE
#  VORBISFILE_LIBRARIES - libraries to link against VORBISFILE
#  VORBISFILE_FOUND - true if VORBISFILE has been found and can be used

find_path(VORBISFILE_INCLUDE_DIR vorbis/vorbisfile.h
  HINTS
    ENV VORBISFILEDIR
  PATH_SUFFIXES include
  PATHS 
  "$ENV{PROGRAMFILES}/../Program\ Files/Vorbis"
  ~/Library/Frameworks
  /Library/Frameworks
  /opt
  /usr/include/
)

find_library(VORBISFILE_LIBRARY 
	NAMES vorbisfile
  HINTS
    ENV VORBISFILEDIR
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64 lib-msvc110
  PATHS
  "$ENV{PROGRAMFILES}/../Program\ Files/VPX"
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/lib/
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VORBISFILE
                                  REQUIRED_VARS VORBISFILE_INCLUDE_DIR VORBISFILE_LIBRARY)

mark_as_advanced(VORBISFILE_LIBRARY VORBISFILE_INCLUDE_DIR)
