ADD_DEFINITIONS(-DSAC_WEB=1 -DSAC_USE_VBO=1 -DSAC_ENABLE_LOG=1)

set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER emcc)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qunused-arguments -O2 --jcache")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments -std=c++0x -O2 --jcache")

add_definitions(-D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)

set(WEB_BUILD 1)

SET (SAC_LIB_TYPE SHARED)

function (others_specific_executables)
endfunction()

function (postbuild_specific_actions)
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} PRE_LINK
        COMMAND rm -rf assets
        COMMAND mkdir assets
        COMMAND cp -r ${PROJECT_SOURCE_DIR}/assets/* ${PROJECT_SOURCE_DIR}/assetspc/* assets
        COMMAND rm assets/*pvr* assets/*pkm*
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building 'assets' folder"
    )
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND EMCC_DEBUG=1 emcc --llvm-lto 1 -O2 -s WARN_ON_UNDEFINED_SYMBOLS=1 -s ALLOW_MEMORY_GROWTH=1
        ${CMAKE_BINARY_DIR}/${EXECUTABLE_NAME} -o ${PROJECT_NAME}.html
         --preload-file assets

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building emscripten HTML"
    )
endfunction()

function (import_specific_libs)
endfunction()
