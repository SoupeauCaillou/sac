set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/sac/build/cmake/toolchains/android.toolchain.cmake)

ADD_DEFINITIONS(-DSAC_MOBILE=1)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -W")
set(CXX_FLAGS_DEBUG "-g -DSAC_ENABLE_LOG -O0")
set(CXX_FLAGS_RELEASE "")

set(MOBILE_BUILD 1)

SET (SAC_LIB_TYPE SHARED)

include_directories(${PROJECT_SOURCE_DIR}/sac/libs/)

#enable GDB debug in debug mode
if (BUILD_TARGET STREQUAL "DEBUG")
    # thanks to http://www.rojtberg.net/465/debugging-native-code-with-ndk-gdb-using-standalone-cmake-toolchain/

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
