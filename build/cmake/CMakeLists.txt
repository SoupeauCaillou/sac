# build library
file(
        GLOB_RECURSE
        source_files
        ../../base/* ../../systems/* ../../systems/* ../../api/linux/* ../../util/* ../../steering/* ../../libs/tinyxml2/tinyxml2.cpp ../../libs/iniparser/*
)

file(
        GLOB_RECURSE
        opt_source_files
        ../../libs/libpng/jni/*.c ../../libs/jsoncpp-src-0.6.0-rc2/src/lib_json/*.cpp ../../libs/enet/*.c
)


file(
        GLOB_RECURSE
        texture_packer_source_files
        ../../tools/texture_packer/*
)

include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/libpng/jni)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/tinyxml2/)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/iniparser/src)

if (EMSCRIPTEN_BUILD)
add_library(sac ${source_files})
else()
add_library(sac ${source_files} ${opt_source_files})
add_definitions( -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DHAS_GETHOSTBYADDR_R=1 -DHAS_GETHOSTBYNAME_R=1 -DHAS_POLL=1 -DHAS_FCNTL=1 -DHAS_INET_PTON=1 -DHAS_INET_NTOP=1 -DHAS_MSGHDR_FLAGS=1 -DHAS_SOCKLEN_T=1 )

# build test app
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/enet/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/sqrat/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/squirrel/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/jsoncpp-src-0.6.0-rc2/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/iniparser/src)

file(
	GLOB_RECURSE
	texture_packer_source
	../../tools/texture_packer/*.cpp	../../tools/texture_packer/*.h
)

add_executable(texture_packer ${texture_packer_source})
target_link_libraries(texture_packer sac z)

file(
	GLOB_RECURSE
	lobby_server_source_files
	../../tools/lobby/*.cpp	../../tools/lobby/*.h
)
add_executable(lobby_server ${lobby_server_source_files})
target_link_libraries(lobby_server sac z)

include_directories(/usr/include/unittest++/)
file(
        GLOB_RECURSE
        test_source_files
        ../../tests/*
)

add_executable(sac_tests ${test_source_files})
target_link_libraries(sac_tests sac GLEW glfw openal sqlite3  vorbisfile vorbis z UnitTest++)
add_custom_command(TARGET sac_tests POST_BUILD COMMAND sac_tests)
endif()
