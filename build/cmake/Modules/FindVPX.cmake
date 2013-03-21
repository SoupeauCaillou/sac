#name in uppercase
set(NAME "VPX")

#names of the 'physic' lib directory
set(DIR_POSSIBLE_NAMES vpx VPX)

#headers' directory name
set(HEADER_DIR include)

#libraries' directory name
set(LIBRARY_DIR lib64 lib libs64 libs libs/Win32 libs/Win64 lib/x64)

#headers' name (.h)
set(HEADER_NAMES vpx/vp8.h)

#libraries' name (.lib)
set(LIBRARY_POSSIBLE_NAMES vpx vpxdll vpxmt)

######################### GENERIC PART #################################
################### It shouldn't be modified ###########################
set(GENERALDIR_POSSIBLE_NAMES 
		"$ENV{PROGRAMFILES}/../Program\ Files"
		~/Library/Frameworks
		/Library/Frameworks
		/opt
		
		#this below is 'sac' related
		"${CMAKE_SOURCE_DIR}/../sac_windows_deps"
		$ENV{SAC_LIBS_DIR})

#search the directory real name if exist
foreach(commondir ${GENERALDIR_POSSIBLE_NAMES})
	foreach(dirname ${DIR_POSSIBLE_NAMES})
		
		if (EXISTS "${commondir}/${dirname}")
			set(MIGHT_LOCATION "${commondir}/${dirname}")
		endif()
	endforeach()
endforeach()

find_path(${NAME}_INCLUDE_DIR ${HEADER_NAMES}
	HINTS
		$ENV{${NAME}DIR}
	PATH_SUFFIXES
		${HEADER_DIR}
	PATHS 
		${MIGHT_LOCATION}
)

find_library(${NAME}_LIBRARY 
	NAMES
		${LIBRARY_POSSIBLE_NAMES}
	HINTS
		$ENV{${NAME}DIR}
	PATH_SUFFIXES
		${LIBRARY_DIR}
	PATHS 
		${MIGHT_LOCATION}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${NAME}
      REQUIRED_VARS ${NAME}_INCLUDE_DIR ${NAME}_LIBRARY)

mark_as_advanced(${NAME}_LIBRARY ${NAME}_INCLUDE_DIR)
