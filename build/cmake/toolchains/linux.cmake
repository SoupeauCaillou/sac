ADD_DEFINITIONS(-DSAC_DESKTOP=1 -DSAC_ENABLE_LOG=1 -DSAC_INGAME_EDITORS=1
    -DSAC_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/" -DENABLE_PROFILING=1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -W -g -O0")

set(DESKTOP_BUILD 1)

SET (SAC_LIB_TYPE STATIC)

function (others_specific_executables)
    #texture packer
    file(
        GLOB_RECURSE texture_packer_source_files
        ${PROJECT_SOURCE_DIR}/sac/tools/texture_packer/*.cpp
        ${PROJECT_SOURCE_DIR}/sac/tools/texture_packer/*.h
    )
    add_executable(texture_packer ${texture_packer_source_files})
    target_link_libraries(texture_packer sac)
endfunction()

function (postbuild_specific_actions)
endfunction()

function (import_specific_libs)
    check_and_link_libs("sac" VPX)
endfunction()
