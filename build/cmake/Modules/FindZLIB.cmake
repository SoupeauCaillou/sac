# - Find the ZLib libarary
# This module defines the following variables:
#  ZLIB_INCLUDE_DIRS - include directories for ZLIB
#  ZLIB_LIBRARIES - libraries to link against ZLIB
#  ZLIB_FOUND - true if ZLIB has been found and can be used

#=============================================================================
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

file(
	GLOB
	MIGHT_LOCATION
		"$ENV{PROGRAMFILES}/../Program\ Files/zlib"
		~/Library/Frameworks
		/Library/Frameworks
		/opt
)
	
find_path(ZLIB_INCLUDE_DIR zlib.h 
	HINTS
		ENV ZLIBDIR
	PATH_SUFFIXES
		include
	PATHS 
		${MIGHT_LOCATION}
)

find_library(ZLIB_LIBRARY 
	NAMES
		z zlib zdll zlib1 zlibd zlibd1
	HINTS
		ENV ZLIBDIR
	PATH_SUFFIXES
		lib64 lib libs64 libs libs/Win32 libs/Win64
	PATHS 
		${MIGHT_LOCATION}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZLIB
                                  REQUIRED_VARS ZLIB_INCLUDE_DIR ZLIB_LIBRARY)

mark_as_advanced(ZLIB_LIBRARY ZLIB_INCLUDE_DIR)
