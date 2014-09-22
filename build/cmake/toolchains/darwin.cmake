ADD_DEFINITIONS(-DSAC_DESKTOP=1 -DSAC_ENABLE_LOG=1 -DSAC_INGAME_EDITORS=1
    -DSAC_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -O0")
set(CXX_FLAGS_DEBUG "-Wall -W -g")

set(DESKTOP_BUILD 1)

SET (SAC_LIB_TYPE STATIC)

function (get_platform_dependent_sources)
    file(
        GLOB_RECURSE platform_source_files
        ${GAME_SOURCE_DIR}/sac/app/*
        ${GAME_SOURCE_DIR}/sac/api/linux/*
        ${GAME_SOURCE_DIR}/platforms/default/api/*
    )
    set (platform_source_files ${platform_source_files} PARENT_SCOPE)
endfunction()

function (others_specific_executables)
    check_and_link_libs("sac" GL)
endfunction()

function (postbuild_specific_actions)
endfunction()

function (import_specific_libs)
   #set(GUI_TYPE MACOSX_BUNDLE)
   include_directories ( /Developer/Headers/FlatCarbon )
   find_library(COCOA_LIBRARY Cocoa)
   find_library(OPENGL_LIBRARY OpenGL )
   find_library(IOKIT_LIBRARY IOKit )
   mark_as_advanced (COCOA_LIBRARY
                     OPENGL_LIBRARY
                     IOKIT_LIBRARY)
   set(EXTRA_LIBS ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY} c)
   target_link_libraries(sac ${EXTRA_LIBS})
endfunction()
