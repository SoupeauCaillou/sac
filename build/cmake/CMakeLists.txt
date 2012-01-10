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

add_library(sac ${base_source_files} ${systems_source_files})

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
target_link_libraries(sac_tests sac UnitTest++)

add_custom_command(
	TARGET sac_tests
	POST_BUILD
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/sac_tests)
