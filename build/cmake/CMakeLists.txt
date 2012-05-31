# build library
file(
        GLOB_RECURSE
        source_files
        ../../base/* ../../systems/* ../../systems/* ../../api/linux/* ../../util/* ../../libs/libpng/jni/*.c
)

file(
        GLOB_RECURSE
        texture_packer_source_files
        ../../tools/texture_packer/*
)


add_library(sac ${source_files})

# build test app
file(
	GLOB_RECURSE
	tests_files
	../../tests/*
)
include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/libpng/jni)
include_directories("/usr/include/unittest++/")
add_executable(sac_tests ${tests_files})
target_link_libraries(sac_tests sac UnitTest++ GLEW glfw openal sndfile vorbis z)
add_executable(texture_packer ${texture_packer_source_files})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/sac/bin)
target_link_libraries(texture_packer png)
add_custom_command(
	TARGET sac_tests
	POST_BUILD
	COMMAND ${PROJECT_BINARY_DIR}/sac/bin/sac_tests)
