ADD_DEFINITIONS(-DSAC_WEB=1 -DSAC_USE_VBO=1 -DSAC_ENABLE_LOG=1)

set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER emcc)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qunused-arguments --jcache")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments -std=c++0x --jcache")

add_definitions(-D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)

set(WEB_BUILD 1)

SET (SAC_LIB_TYPE SHARED)

function (get_platform_dependent_sources)
    file(
        GLOB_RECURSE platform_source_files
        ${PROJECT_SOURCE_DIR}/sac/app/*
        ${PROJECT_SOURCE_DIR}/sac/api/linux/*
        ${PROJECT_SOURCE_DIR}/platforms/default/api/*
    )
    set (platform_source_files ${platform_source_files} PARENT_SCOPE)
endfunction()

function (others_specific_executables)
endfunction()

function (postbuild_specific_actions)
    execute_process(COMMAND date OUTPUT_VARIABLE DATE_VAR)
    STRING(REGEX REPLACE "\n" "" DATE_VAR ${DATE_VAR} )
    execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE COMMIT_VAR)
    STRING(REGEX REPLACE "\n" "" COMMIT_VAR ${COMMIT_VAR} )
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} PRE_LINK
        COMMAND rm -rf assets
        COMMAND mkdir assets
        COMMAND cp -r ${PROJECT_SOURCE_DIR}/assets/* ${PROJECT_SOURCE_DIR}/assetspc/* assets
        COMMAND find assets/ -name '*pvr*' -or -name '*pkm*' -exec rm -r {} '\;'
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building 'assets' folder"
    )
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND EMCC_DEBUG=1 emcc --llvm-lto 1 -O2 -s TOTAL_MEMORY=1073741824 -s VERBOSE=1 -s WARN_ON_UNDEFINED_SYMBOLS=1
        ${CMAKE_BINARY_DIR}/${EXECUTABLE_NAME} -o ${PROJECT_NAME}.html
         --preload-file assets
        COMMAND sed -i "s/Emscripten-Generated\ Code/${PROJECT_NAME} - ${DATE_VAR} - ${COMMIT_VAR}/" ${PROJECT_NAME}.html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building emscripten HTML"
        VERBATIM
    )
endfunction()

function (import_specific_libs)
endfunction()
