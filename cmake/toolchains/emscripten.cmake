ADD_DEFINITIONS(-DSAC_WEB=1)

set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER emcc)

if (${BUILD_TARGET} STREQUAL "DEBUG")
    set(EMSCRIPTEN_OPTIONS "-O0 --llvm-lto 0 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s ASSERTIONS=1 -s ASM_JS=1 --closure 0 -s TOTAL_MEMORY=1073741824 -s WARN_ON_UNDEFINED_SYMBOLS=1")
    add_definitions(-DSAC_ENABLE_LOG=1)
    add_definitions(-DSAC_DEBUG=1)
    add_definitions(-DSAC_INGAME_EDITORS=1)
else()
    set(EMSCRIPTEN_OPTIONS "-s USE_SDL=2 -s USE_SDL_IMAGE=2  -O2 --llvm-lto 1 --llvm-lto 1 -s ASM_JS=1 --closure 0 -s TOTAL_MEMORY=67108864 -s ERROR_ON_UNDEFINED_SYMBOLS=1 -s PRECISE_I64_MATH=0")
endif()

STRING(REPLACE " " ";" EMSCRIPTEN_OPTIONS_LIST ${EMSCRIPTEN_OPTIONS})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qunused-arguments  ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments -std=c++0x ")
set(CXX_FLAGS_DEBUG ${EMSCRIPTEN_OPTIONS})
set(CXX_FLAGS_RELEASE ${EMSCRIPTEN_OPTIONS})

add_definitions(-D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)

set(WEB_BUILD 1)

SET (SAC_LIB_TYPE SHARED)

function (get_platform_dependent_sources)
    file(
        GLOB_RECURSE platform_source_files
        ${GAME_SOURCE_DIR}/sac/app/*
        ${GAME_SOURCE_DIR}/sac/api/linux/*
        ${GAME_SOURCE_DIR}/sac/api/default/*
        ${GAME_SOURCE_DIR}/sac/api/sdl/*
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
        COMMAND cp -r ${GAME_SOURCE_DIR}/assets/* ${GAME_SOURCE_DIR}/res/* assets
        COMMAND find assets/ -name '*pvr*' -or -name '*pkm*' -exec rm -r {} '\;'
        COMMAND rm -r assets/ldpi assets/mdpi
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building 'assets' folder"
    )
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND EMCC_DEBUG=1 emcc ${EMSCRIPTEN_OPTIONS_LIST} ${CMAKE_BINARY_DIR}/${EXECUTABLE_NAME} ${CMAKE_BINARY_DIR}/libsac.so -o ${PROJECT_NAME}.html
         --preload-file assets
        COMMAND sed -i "s/Emscripten-Generated\ Code/${PROJECT_NAME} - ${DATE_VAR} - ${COMMIT_VAR}/" ${PROJECT_NAME}.html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building emscripten HTML"
        VERBATIM
    )
endfunction()

function (import_specific_libs)
endfunction()
