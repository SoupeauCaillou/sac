ADD_DEFINITIONS(-DSAC_DESKTOP=1 -DSAC_ENABLE_LOG=1 -DSAC_INGAME_EDITORS=1
    -DSAC_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /MP /D NOMINMAX /D NOGDI /D WIN32_LEAN_AND_MEAN")
set(CXX_FLAGS_DEBUG "/W4")

set(DESKTOP_BUILD 1)

SET (SAC_LIB_TYPE STATIC)

function (others_specific_executables)
endfunction()

function (postbuild_specific_actions)
    #copy the libs .dll near the .exe
    set(SAC_DLL_POSSIBLE_DIRS $ENV{SAC_DLLS_DIR} "${PROJECT_SOURCE_DIR}/../sac_dlls_dep")
    foreach (SAC_DLLS_DIR ${SAC_DLL_POSSIBLE_DIRS})
        if (EXISTS ${SAC_DLLS_DIR})
            set(BIN_DIR "${PROJECT_BINARY_DIR}/platforms/default/")
            foreach (BUILD_TYPE "Debug" "Release")
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SAC_DLLS_DIR}"
                    "${BIN_DIR}/${BUILD_TYPE}"
                )
            endforeach()
        endif()
    endforeach()
endfunction()

function (import_specific_libs)
    target_link_libraries (sac opengl32 Winmm)
endfunction()
