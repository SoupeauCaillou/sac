ADD_DEFINITIONS(-DSAC_WEB=1 -DSAC_USE_VBO)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++0x -g -O0 -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -O0 -m32")
set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER emcc)

add_definitions(-D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)

set(WEB_BUILD 1)

function (others_specific_executables)
endfunction()

function (postbuild_specific_actions)
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND EMCC_DEBUG=1 emcc -s WARN_ON_UNDEFINED_SYMBOLS=1
        ${CMAKE_BINARY_DIR}/${EXECUTABLE_NAME} -o /tmp/${PROJECT_NAME}.html
         --preload-file assets

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building emscripten HTML"
    )
endfunction()

function (import_specific_libs)
endfunction()
