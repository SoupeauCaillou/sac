set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/sac/build/cmake/Modules")

include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/AntTweakBar/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/libpng/jni)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/tinyxml2/)
link_directories (${PROJECT_SOURCE_DIR}/sac/libs/AntTweakBar/lib)
#include_directories(${PROJECT_SOURCE_DIR}/sac/libs/glog-gflags)

file(
	GLOB_RECURSE
	source_files
	../../app/* ../../base/* ../../systems/* ../../api/linux/* ../../util/*
	../../steering/* ../../libs/tinyxml2/tinyxml2.cpp
	#../../libs/glog-gflags/*.cc
)

file(
	GLOB_RECURSE
	opt_source_files
	../../libs/libpng/jni/*.c ../../libs/jsoncpp-src-0.6.0-rc2/src/lib_json/*.cpp
	 ../../libs/enet/*.c
)	

#how to build sac
if (EMSCRIPTEN_BUILD)
	add_library(sac ${source_files})
else()
	#build external libs
	add_library(sac ${source_files} ${opt_source_files})

	add_definitions(-DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DHAS_GETHOSTBYADDR_R=1 -DHAS_GETHOSTBYNAME_R=1 -DHAS_POLL=1 -DHAS_FCNTL=1 -DHAS_INET_PTON=1 -DHAS_INET_NTOP=1 -DHAS_MSGHDR_FLAGS=1 -DHAS_SOCKLEN_T=1 )

	include_directories(${PROJECT_SOURCE_DIR}/sac/libs/enet/include)
	include_directories(${PROJECT_SOURCE_DIR}/sac/libs/sqrat/include)
	include_directories(${PROJECT_SOURCE_DIR}/sac/libs/squirrel/include)
	include_directories(${PROJECT_SOURCE_DIR}/sac/libs/jsoncpp-src-0.6.0-rc2/include)
	

	find_package (GLEW)
	if (GLEW_FOUND)
		  include_directories(${GLEW_INCLUDE_DIR})
      target_link_libraries (sac ${GLEW_LIBRARY})
	endif ()

	find_package (GLFW)
	if (GLFW_FOUND)
	  include_directories(${GLFW_INCLUDE_DIR})
      target_link_libraries (sac ${GLFW_LIBRARY})
	endif ()

	find_package (OpenAL)
	if (OPENAL_FOUND)
		include_directories(${OPENAL_INCLUDE_DIR})
        target_link_libraries (sac ${OPENAL_LIBRARY})
	endif ()

	find_package (Sqlite3)
	if (SQLITE3_FOUND)
	  include_directories(${SQLITE3_INCLUDE_DIRS})
	  target_link_libraries (${PROJECT_NAME} ${SQLITE3_LIBRARIES})
	endif ()

	find_package (VorbisFile)
	if (VORBISFILE_FOUND)
	  include_directories(${VORBISFILE_INCLUDE_DIR})
	  target_link_libraries (sac ${VORBISFILE_LIBRARY})
    endif ()

	find_package (Vorbis)
	if (VORBIS_FOUND)
	  include_directories(${VORBIS_INCLUDE_DIRS})
	  target_link_libraries (${PROJECT_NAME} ${VORBIS_LIBRARIES})
	endif ()

	find_package (VPX)
	if (VPX_FOUND)
	  include_directories(${VPX_INCLUDE_DIR})
	  target_link_libraries (sac ${VPX_LIBRARY})
	endif ()

	find_package (ZLIB)
	if (ZLIB_FOUND)
	  include_directories(${ZLIB_INCLUDE_DIR})
      target_link_libraries (sac ${ZLIB_LIBRARY})
	endif ()

	find_package (AntTweakBar)
	if (ANTTWEAKBAR_FOUND)
	  include_directories(${ANTTWEAKBAR_INCLUDE_DIR})
      target_link_libraries (sac ${ANTTWEAKBAR_LIBRARY})
	endif ()

    target_link_libraries(sac)




	#texture packer
		file(
			GLOB_RECURSE
			texture_packer_source_files
			../../tools/texture_packer/*.cpp	../../tools/texture_packer/*.h
		)
		add_executable(texture_packer ${texture_packer_source_files})
		target_link_libraries(texture_packer sac z)
	
	#lobby server
		file(
			GLOB_RECURSE
			lobby_server_source_files
			../../tools/lobby/*.cpp	../../tools/lobby/*.h
		)
		add_executable(lobby_server ${lobby_server_source_files})
		target_link_libraries(lobby_server sac z)

	#unit tests, if installed
		# find_package (UnitTest++)
		# if (UNITTEST_FOUND)
			# include_directories(${UNITTEST_INCLUDE_DIR})
			# file(
					# GLOB_RECURSE
					# test_source_files
					# ../../tests/*
			# )
			# add_executable(sac_tests ${test_source_files})
			
			# sac_tests dependences
			# target_link_libraries(sac_tests sac GLEW glfw openal sqlite3 vorbisfile vorbis z UnitTest++)
			
			# launch sac_tests after each build
			# add_custom_command(TARGET sac_tests POST_BUILD COMMAND sac_tests)
		# endif ()
endif()
