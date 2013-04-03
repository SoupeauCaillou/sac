ADD_DEFINITIONS(-DSAC_DESKTOP -DSAC_DEBUG=1 -DSAC_ENABLE_LOG=1 -DSAC_INGAME_EDITORS=1
    -DSAC_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -W -g -O0")

set(DESKTOP_BUILD 1)

SET (SAC_LIB_TYPE STATIC)

function (others_specific_executables)
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
