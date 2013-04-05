set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/sac/build/cmake/toolchains/android.toolchain.cmake)

ADD_DEFINITIONS(-DSAC_MOBILE=1 -DSAC_DEBUG=1 -DSAC_ENABLE_LOG=1
    -DSAC_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -W -g -O0")

set(MOBILE_BUILD 1)

SET (SAC_LIB_TYPE SHARED)

include_directories(${PROJECT_SOURCE_DIR}/sac/libs/)

function (others_specific_executables)
endfunction()

function (postbuild_specific_actions)
endfunction()

function (import_specific_libs)
endfunction()
