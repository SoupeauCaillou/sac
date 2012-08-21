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
include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/libpng/jni)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/sqrat/include)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/squirrel/include)
