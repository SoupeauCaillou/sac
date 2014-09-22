ADD_DEFINITIONS(-DSAC_DESKTOP=1 -DSAC_ENABLE_LOG=1 -DSAC_INGAME_EDITORS=1 -DGLM_FORCE_RADIANS
    -DSAC_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /D NOMINMAX /D NOGDI /D WIN32_LEAN_AND_MEAN /W4")

set(DESKTOP_BUILD 1)

SET (SAC_LIB_TYPE STATIC)

function (get_platform_dependent_sources)
    file(
        GLOB_RECURSE platform_source_files PARENT_SCOPE
        ${GAME_SOURCE_DIR}/sac/app/*
        ${GAME_SOURCE_DIR}/sac/api/linux/* #oops! should be renamed
        ${GAME_SOURCE_DIR}/sac/api/windows/*
        ${GAME_SOURCE_DIR}/platforms/default/api/*
    )
    set (platform_source_files ${platform_source_files} PARENT_SCOPE)
endfunction()

function (others_specific_executables)
endfunction()

function (postbuild_specific_actions)
    #copy the libs .dll near the .exe
    set(SAC_DLL_POSSIBLE_DIRS $ENV{SAC_DLLS_DIR} "${GAME_SOURCE_DIR}/../sac_dlls_dep")
    foreach (SAC_DLLS_DIR ${SAC_DLL_POSSIBLE_DIRS})
        if (EXISTS ${SAC_DLLS_DIR})
            message("Copy dll near the exe after build from directory ${SAC_DLLS_DIR} to directory ${CMAKE_BINARY_DIR}/[Debug|Release]!")
            foreach (BUILD_TYPE "Debug" "Release")
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SAC_DLLS_DIR}"
                    ${CMAKE_BINARY_DIR}/${BUILD_TYPE}
                )
            endforeach()
        endif()
    endforeach()
endfunction()

function (import_specific_libs)
    target_link_libraries (sac opengl32 Winmm)

    if (NETWORK_BUILD)
        target_link_libraries("sac" ws2_32)
    endif()
endfunction()
