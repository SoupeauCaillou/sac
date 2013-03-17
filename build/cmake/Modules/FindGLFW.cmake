# - Find GLFW libraries by Gautier :-).
# This module defines the following variables:
#  GLFW_INCLUDE_DIRS - include directories for GLFW
#  GLFW_LIBRARIES - libraries to link against GLFW
#  GLFW_FOUND - true if GLFW has been found and can be used

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
    "$ENV{PROGRAMFILES}/../Program\ Files/glfw"
    ~/Library/Frameworks
    /Library/Frameworks
    /opt
    /usr/include/
)
	
find_path(GLFW_INCLUDE_DIR GL/glfw.h 
  HINTS
    ENV GLFWDIR
  PATH_SUFFIXES
    include/GL include
  PATHS 
    ${MIGHT_LOCATION}
)


find_library(GLFW_LIBRARY 
  NAMES
    GLFW GLFWDLL glfw
  HINTS
    ENV GLFWDIR
  PATH_SUFFIXES
    lib64 lib libs64 libs libs/Win32 libs/Win64 lib-msvc110
  PATHS
    ${MIGHT_LOCATION}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW
                                  REQUIRED_VARS GLFW_INCLUDE_DIR GLFW_LIBRARY)

mark_as_advanced(GLFW_LIBRARY GLFW_INCLUDE_DIR)
