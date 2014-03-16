set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/sac/build/cmake/toolchains/android.toolchain.cmake)

ADD_DEFINITIONS(-DSAC_MOBILE=1)
ADD_DEFINITIONS(-DSAC_USE_VBO=1)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -W")
set(CXX_FLAGS_DEBUG "-g -DSAC_ENABLE_LOG -O0")
set(CXX_FLAGS_RELEASE "")

set(MOBILE_BUILD 1)

SET (SAC_LIB_TYPE SHARED)

include_directories(${PROJECT_SOURCE_DIR}/sac/libs/)

#######################################################################
###########################Enable GDB debug###########################
#######################################################################
if (0 AND BUILD_TARGET STREQUAL "DEBUG")
    # thanks to http://www.rojtberg.net/465/debugging-native-code-with-ndk-gdb-using-standalone-cmake-toolchain/
    if (NOT ANDROID_NDK)
        set (ANDROID_NDK $ENV{ANDROID_NDK})
    endif()

    # 1. generate Android.mk
    file(WRITE ${PROJECT_SOURCE_DIR}/jni/Android.mk "APP_ABI := ${ANDROID_NDK_ABI_NAME}\n")

    # 2. generate gdb.setup
    get_directory_property(INCLUDE_DIRECTORIES DIRECTORY ${PROJECT_SOURCE_DIR} INCLUDE_DIRECTORIES)
    string(REGEX REPLACE ";" " " INCLUDE_DIRECTORIES "${INCLUDE_DIRECTORIES}")
    file(WRITE ${PROJECT_SOURCE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/gdb.setup "set solib-search-path ${PROJECT_SOURCE_DIR}/obj/local/${ANDROID_NDK_ABI_NAME}\n")
    file(APPEND ${PROJECT_SOURCE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/gdb.setup "directory ${INCLUDE_DIRECTORIES}\n")

    # 3. copy gdbserver executable
    file(COPY ${ANDROID_NDK}/prebuilt/android-arm/gdbserver/gdbserver DESTINATION ${PROJECT_SOURCE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/)
endif()


#######################################################################
#########Generate plugins list needed for the application##############
#######################################################################
    #clean file
    file (WRITE "${PROJECT_SOURCE_DIR}/res/values/plugins.xml" 
        "<?xml version='1.0' encoding='UTF-8'?>\n<resources>\n\t<string-array name='plugins_list'>\n")

    # get the list of plugins bundled with the APK, in project.properties file
    execute_process(
        COMMAND grep android.library.reference project.properties
        COMMAND grep -v SacFramework
        COMMAND cut -d= -f 2
        OUTPUT_VARIABLE PLUGINS_LIST
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
    if (PLUGINS_LIST)
        string(REPLACE "\n" ";" PLUGINS_LIST ${PLUGINS_LIST})
    endif()

    # and find package/class names for each of these plugins
    foreach (PLUGIN_PATH ${PLUGINS_LIST})
        # Assume that package name convention is 
        # net.damsy.soupeaucaillou.[plugin folder in lowercase and without 'sac' prefix ]
        string (TOLOWER "${PLUGIN_PATH}" plugin_path)
        string(REGEX REPLACE ".+/sac(.*)$" "net.damsy.soupeaucaillou.\\1" PACKAGE_NAME ${plugin_path})
        
        # Assume that plugin name convention is
        # [plugin folder with 'Plugin' suffix]
        string(REGEX REPLACE ".*/(.*)$" "\\1Plugin" CLASS_NAME ${PLUGIN_PATH})

        file (APPEND "${PROJECT_SOURCE_DIR}/res/values/plugins.xml"
            "\t\t<item>${PACKAGE_NAME}.${CLASS_NAME}</item>\n")
    endforeach()

    file (APPEND "${PROJECT_SOURCE_DIR}/res/values/plugins.xml"
        "\t</string-array>\n</resources>\n")


#######################################################################
#######Declaration of functions needed by the main CMakeLists.txt######
#######################################################################
function (get_platform_dependent_sources)
    file(
        GLOB_RECURSE platform_source_files
        ${PROJECT_SOURCE_DIR}/sac/api/android/*
        ${PROJECT_SOURCE_DIR}/sources/api/android/*
        ${PROJECT_SOURCE_DIR}/sac/android/*
        ${PROJECT_SOURCE_DIR}/platforms/android/*.cpp
    )
    set (platform_source_files ${platform_source_files} PARENT_SCOPE)
endfunction()


function (others_specific_executables)
endfunction()

function (postbuild_specific_actions)
    if (USE_GRADLE)
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
    endif()

    # 4. copy lib to obj
    add_custom_command(
        TARGET "sac" POST_BUILD 
        COMMAND mkdir -p ${PROJECT_SOURCE_DIR}/obj/local/${ANDROID_NDK_ABI_NAME}/
        COMMAND cp ${PROJECT_SOURCE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/libsac.so ${PROJECT_SOURCE_DIR}/obj/local/${ANDROID_NDK_ABI_NAME}/
    )

    # 5. strip symbols
    add_custom_command(
        TARGET "sac" POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${PROJECT_SOURCE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/libsac.so
    )
endfunction()

function (import_specific_libs)
endfunction()
