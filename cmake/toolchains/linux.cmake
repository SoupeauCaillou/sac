add_definitions(-DSAC_DESKTOP=1)
add_definitions(-DSAC_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")

# Enable '-D_GLIBCXX_DEBUG' to debug stl containers related issues

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread -g -Wall -W -Werror")
set(CXX_FLAGS_DEBUG "-O0 -Wno-unused-parameter")
set(CXX_FLAGS_RELEASE "-O3 -fconstexpr-depth=10000")

if (${BUILD_TARGET} STREQUAL "DEBUG")
    add_definitions(-DSAC_ENABLE_LOG=1)
    add_definitions(-DSAC_DEBUG=1)
    add_definitions(-DSAC_INGAME_EDITORS=1)
    #add_definitions(-D_GLIBCXX_DEBUG)

    set(INGAME_EDITOR "ON")
endif()
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_definitions(-Qunused-arguments -Wno-tautological-compare)



    # workaround bug http://llvm.org/bugs/show_bug.cgi?id=12730
    # message ("clang compiler detected -> workaround bug #12730")
    # add_definitions(-D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)
endif()

set(DESKTOP_BUILD 1)

SET (SAC_LIB_TYPE STATIC)




function (get_platform_dependent_sources)
    file(
        GLOB_RECURSE platform_source_files
        ${SAC_SOURCE_DIR}/app/*
        ${SAC_SOURCE_DIR}/api/linux/*
        ${SAC_SOURCE_DIR}/api/sdl/*
        ${GAME_SOURCE_DIR}/platforms/default/api/*
    )
    set (platform_source_files ${platform_source_files} PARENT_SCOPE)
endfunction()

function (others_specific_executables)
    #texture packer
    file(
        GLOB_RECURSE texture_packer_source_files
        ${SAC_SOURCE_DIR}/tools/texture_packer/*.cpp
        ${SAC_SOURCE_DIR}/tools/texture_packer/*.h
    )
    add_executable(texture_packer ${texture_packer_source_files})
    target_link_libraries(texture_packer sac rt png)
endfunction()

function (postbuild_specific_actions)
    target_link_libraries(${EXECUTABLE_NAME} GL)
endfunction()

function (import_specific_libs)
    check_and_link_libs("sac" VPX GL)
    if (SAC_HAVE_CURL)
        add_definitions(sac -DSAC_HAVE_CURL=1)
        check_and_link_libs("sac" CURL)
    endif()
endfunction()
