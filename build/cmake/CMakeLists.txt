# build library
file(
        GLOB_RECURSE
        source_files
        ../../base/* ../../systems/* ../../systems/* ../../api/linux/* ../../util/* ../../steering/* ../../libs/libpng/jni/*.c ../../libs/jsoncpp-src-0.6.0-rc2/src/lib_json/*.cpp
)

file(
        GLOB_RECURSE
        texture_packer_source_files
        ../../tools/texture_packer/*
)

add_library(sac ${source_files})

# build test app
include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/libpng/jni)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/sqrat/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/squirrel/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/jsoncpp-src-0.6.0-rc2/include)

file(
	GLOB_RECURSE
	texture_packer_source
	../../tools/texture_packer/*.cpp	../../tools/texture_packer/*.h
)

add_executable(texture_packer ${texture_packer_source})
target_link_libraries(texture_packer sac z)
