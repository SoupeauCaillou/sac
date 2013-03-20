# - Find Sqlite3 library by Gautier :-).
# This module defines the following variables:
#  SQLITE3_INCLUDE_DIRS - include directories for SQLITE3
#  SQLITE3_LIBRARIES - libraries to link against SQLITE3
#  SQLITE3_FOUND - true if SQLITE3 has been found and can be used


file(
	GLOB
	MIGHT_LOCATION
		"$ENV{PROGRAMFILES}/../Program\ Files/Sqlite3"
		~/Library/Frameworks
		/Library/Frameworks
		/opt
)
	
find_path(SQLITE3_INCLUDE_DIR sqlite3.h
  HINTS
    ENV SQLITE3DIR
  PATH_SUFFIXES
    include/GL include
  PATHS 
    ${MIGHT_LOCATION}
)

find_library(SQLITE3_LIBRARY 
  NAMES
    sqlite3
  HINTS
    ENV SQLITE3DIR
  PATH_SUFFIXES
    lib64 lib libs64 libs libs/Win32 libs/Win64
  PATHS 
    ${MIGHT_LOCATION}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLITE3
                                  REQUIRED_VARS SQLITE3_INCLUDE_DIR SQLITE3_LIBRARY)

mark_as_advanced(SQLITE3_LIBRARY SQLITE3_INCLUDE_DIR)
