# Specific options for android: possibility to disable proprietary plugins. This is mainly used for F-droid build
option(USE_PROPRIETARY_PLUGINS "Use proprietary plugins (plugins within project.properties contains use.proprietary.license=true (such as Google Play plugins)" ON)

if (USE_PROPRIETARY_PLUGINS)
    add_definitions(-DSAC_USE_PROPRIETARY_PLUGINS=1)
endif()

ADD_DEFINITIONS(-DSAC_MOBILE=1 -DDISABLE_NETWORK_SYSTEM=1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -W")
set(CXX_FLAGS_DEBUG "-g -DSAC_ENABLE_LOG -O0")
set(CXX_FLAGS_RELEASE "")

set(MOBILE_BUILD 1)

SET (SAC_LIB_TYPE STATIC)

include_directories(${GAME_SOURCE_DIR}/sac/libs/)

#######################################################################
#######Declaration of functions needed by the main CMakeLists.txt######
#######################################################################
function (get_platform_dependent_sources)
    file(
        GLOB_RECURSE platform_source_files
        ${GAME_SOURCE_DIR}/sac/app/AppSetupIOS.*
        ${GAME_SOURCE_DIR}/sac/api/ios/*
        ${GAME_SOURCE_DIR}/sac/api/linux/AssetAPILinuxImpl.*
        ${GAME_SOURCE_DIR}/sac/api/default/LocalizeAPITextImpl.*
        ${GAME_SOURCE_DIR}/sac/api/linux/MusicAPILinuxOpenALImpl.*
        ${GAME_SOURCE_DIR}/sac/api/linux/SoundAPILinuxOpenALImpl.*
    )
    set (platform_source_files ${platform_source_files} PARENT_SCOPE)
endfunction()


function (others_specific_executables)
endfunction()

function (postbuild_specific_actions)
endfunction()

function (import_specific_libs)
  find_library(CORE_LIBRARY CoreFoundation)
  find_library(OPENAL_LIBRARY OpenAL)
   find_library(OPENAL_LIBRARY OpenAL)
   find_library(OPENGLES_LIBRARY OpenGLES )
   mark_as_advanced (OPENAL_LIBRARY CORE_LIBRARY
                     OPENGLES_LIBRARY)
   set(EXTRA_LIBS ${OPENAL_LIBRARY} ${OPENGLES_LIBRARY} ${CORE_LIBRARY})
   target_link_libraries(sac ${EXTRA_LIBS})
   message("youyouyou" ${EXTRA_LIBS})
endfunction()
