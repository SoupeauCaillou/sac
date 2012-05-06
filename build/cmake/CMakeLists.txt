# build library
file(
        GLOB_RECURSE
        base_source_files
        ../../base/*
)
file(
        GLOB_RECURSE
        systems_source_files
        ../../systems/*
)

file(
        GLOB_RECURSE
        api_source_files
        ../../api/linux/*
)

file(
        GLOB_RECURSE
        texture_packer_source_files
        ../../tools/texture_packer/*
)


add_library(sac ${base_source_files} ${systems_source_files} ${api_source_files})

# build test app
file(
	GLOB_RECURSE
	tests_files
	../../tests/*
)
include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories("/usr/include/unittest++/")
add_executable(sac_tests ${tests_files})
target_link_libraries(sac_tests sac UnitTest++ GLEW glfw openal sndfile vorbis)
add_executable(texture_packer ${texture_packer_source_files})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/sac/bin)
target_link_libraries(texture_packer png)
add_custom_command(
	TARGET sac_tests
	POST_BUILD
	COMMAND ${PROJECT_BINARY_DIR}/sac/bin/sac_tests)
