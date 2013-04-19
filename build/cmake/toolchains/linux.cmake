ADD_DEFINITIONS(-DSAC_DESKTOP=1 -DSAC_DEBUG=1 -DSAC_ENABLE_LOG=1 -DSAC_INGAME_EDITORS=1
    -DSAC_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/" -DENABLE_PROFILING=1 -DSAC_USE_VBO=1)

# Enable '-D_GLIBCXX_DEBUG' to debug stl containers related issues

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread -O0")
set(CXX_FLAGS_DEBUG "-Wall -W -g")

if (${CMAKE_C_COMPILER} MATCHES "(.*)clang")
    # workaround bug http://llvm.org/bugs/show_bug.cgi?id=12730
    message ("clang compiler detected -> workaround bug #12730")
    add_definitions(-D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)
endif()

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
    target_link_libraries(${EXECUTABLE_NAME} GL)
endfunction()

function (import_specific_libs)
    check_and_link_libs("sac" VPX GL)
endfunction()
