# - Find the OpenGL Extension Wrangler Library (GLEW)
# This module defines the following variables:
#  GLEW_INCLUDE_DIRS - include directories for GLEW
#  GLEW_LIBRARIES - libraries to link against GLEW
#  GLEW_FOUND - true if GLEW has been found and can be used

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
    "$ENV{PROGRAMFILES}/../Program\ Files/VPX"
    ~/Library/Frameworks
    /Library/Frameworks
    /opt
    /usr/include/
)
	
find_path(VPX_INCLUDE_DIR vpx/vp8.h 
	HINTS
		ENV VPXDIR
	PATH_SUFFIXES
		include
	PATHS 
		${MIGHT_LOCATION}
)

find_library(VPX_LIBRARY 
	NAMES
		vpx vpxdll vpxmt
	HINTS
		ENV VPXDIR
	PATH_SUFFIXES
		lib64 lib libs64 libs libs/Win32 libs/Win64 lib/x64
	PATHS 
		${MIGHT_LOCATION}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VPX
                                  REQUIRED_VARS VPX_INCLUDE_DIR VPX_LIBRARY)

mark_as_advanced(VPX_LIBRARY VPX_INCLUDE_DIR)
