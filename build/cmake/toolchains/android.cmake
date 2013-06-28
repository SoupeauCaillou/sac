set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/sac/build/cmake/toolchains/android.toolchain.cmake)

ADD_DEFINITIONS(-DSAC_MOBILE=1 -DSAC_ENABLE_LOG=1
    -DSAC_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -O0")
set(CXX_FLAGS_DEBUG "-Wall -W -g")

set(MOBILE_BUILD 1)

SET (SAC_LIB_TYPE SHARED)

include_directories(${PROJECT_SOURCE_DIR}/sac/libs/)

function (others_specific_executables)
endfunction()

function (postbuild_specific_actions)
    add_custom_command(
        TARGET "sac" POST_BUILD
        COMMAND rm -rf tmplibs/ ${PROJECT_SOURCE_DIR}/libs/armeabi-v7a.jar
        COMMAND mkdir -p tmplibs/lib
        COMMAND cp -r ${PROJECT_SOURCE_DIR}/libs/armeabi-v7a tmplibs/lib
        COMMAND rm -r tmplibs/lib/armeabi-v7a/*.a
        COMMAND cd tmplibs && zip -r ${PROJECT_SOURCE_DIR}/libs/armeabi-v7a.jar lib/*
        COMMAND rm -rf tmplibs/
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Creating armeabi.jar. Hack for gradle since it does not support native-lib yet (https://groups.google.com/forum/#!msg/adt-dev/nQobKd2Gl_8/Z5yWAvCh4h4J)"
    )
endfunction()

function (import_specific_libs)
endfunction()
