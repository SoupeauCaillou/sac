ADD_DEFINITIONS(-DSAC_USE_VBO)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++0x -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -O0")
set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER emcc)

set(WEB_BUILD 1)

function (others_specific_executables)
endfunction()

function (postbuild_specific_actions)
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND EMCC_DEBUG=1 emcc -s WARN_ON_UNDEFINED_SYMBOLS=1 -O2 --llvm-lto 1
        ${CMAKE_BINARY_DIR}/linux/${EXECUTABLE_NAME} -o /tmp/${PROJECT_NAME}.html
         --preload-file assets

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building emscripten HTML"
    )
endfunction()

function (import_specific_libs)
endfunction()
