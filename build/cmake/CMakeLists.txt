set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/sac/build/cmake/Modules")

include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/libpng/jni)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/glm)
file(
    GLOB_RECURSE source_files
    ${PROJECT_SOURCE_DIR}/sac/app/*
    ${PROJECT_SOURCE_DIR}/sac/base/*
    ${PROJECT_SOURCE_DIR}/sac/systems/*
    ${PROJECT_SOURCE_DIR}/sac/util/*
    ${PROJECT_SOURCE_DIR}/sac/steering/*
    ${PROJECT_SOURCE_DIR}/sources/*
)

#ensure that a lib is installed and link it to "target" executable
function (check_and_link_libs target)
   #you shouldn't touch that
    foreach(lib ${ARGN})
        string(TOUPPER ${lib} LIB) #in uppercase

        find_package (${lib})
        if (${LIB}_FOUND)
          include_directories(${${LIB}_INCLUDE_DIR})
          target_link_libraries (${target} ${${LIB}_LIBRARY})
        endif ()
    endforeach()
endfunction()


#how to build sac
if (EMSCRIPTEN_BUILD)
    add_library(sac ${source_files})

    add_executable(${PROJECT_NAME}.bc ${source_files})

    target_link_libraries(${PROJECT_NAME}.bc sac)

    check_and_link_libs(${PROJECT_NAME}.bc GLEW glfw openal sqlite3 vorbisfile vorbis
    z png)
else()
    file(
        GLOB_RECURSE opt_source_files
        ${PROJECT_SOURCE_DIR}/sac/libs/libpng/jni/*.c #libpng
        ${PROJECT_SOURCE_DIR}/sac/libs/jsoncpp-src-0.6.0-rc2/src/lib_json/*.cpp #lib json
        ${PROJECT_SOURCE_DIR}/sac/libs/enet/*.c #lib enet
    )

    add_definitions(-DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1
    -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1
    -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1
    -DHAS_POLL=1 -DHAS_FCNTL=1 -DHAS_INET_PTON=1 -DHAS_INET_NTOP=1 -DHAS_MSGHDR_FLAGS=1
    -DHAS_SOCKLEN_T=1 )

    include_directories(${PROJECT_SOURCE_DIR}/sac/libs/enet/include)
    include_directories(${PROJECT_SOURCE_DIR}/sac/libs/jsoncpp-src-0.6.0-rc2/include)


    #include platform dependent APIs
    if (DESKTOP_BUILD)
        file(
            GLOB_RECURSE platform_source_files
            ${PROJECT_SOURCE_DIR}/sac/api/linux/*
            ${PROJECT_SOURCE_DIR}/sources/api/linux/*
        )
    elseif (MOBILE_BUILD)
        file(
            GLOB_RECURSE platform_source_files
            ${PROJECT_SOURCE_DIR}/sac/api/android/*
            ${PROJECT_SOURCE_DIR}/sources/api/android/*
        )
    else()
        message("which platform?")
    endif()

    add_library(sac ${source_files} ${opt_source_files} ${platform_source_files})

    #the necessary libs
    check_and_link_libs("sac" GLEW GLFW OpenAL Sqlite3 VorbisFile Vorbis ZLIB AntTweakBar
    Ogg Tinyxml2)

    #particular libs
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        check_and_link_libs("sac" VPX)
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_libraries (sac opengl32 Winmm)
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
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
    endif ()





    file(
        GLOB_RECURSE source_platforms_files
        ${PROJECT_SOURCE_DIR}/platforms/default/*
    )
    add_executable(${PROJECT_NAME} ${source_platforms_files})
    target_link_libraries(${PROJECT_NAME} sac)

endif()
#######################################################################
#############################Post build actions########################
#######################################################################
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
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
elseif (EMSCRIPTEN_BUILD)
    add_custom_command(
        TARGET ${PROJECT_NAME}.bc POST_BUILD
        COMMAND EMCC_DEBUG=1 emcc -s WARN_ON_UNDEFINED_SYMBOLS=1 -O2 --llvm-lto 1
        ${CMAKE_BINARY_DIR}/linux/${PROJECT_NAME}.bc -o /tmp/${PROJECT_NAME}.html
         --preload-file assets

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building emscripten HTML"
    )
endif()

#######################################################################
#############################others executables########################
#######################################################################

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    #lobby server
    file(
        GLOB_RECURSE lobby_server_source_files
        ${PROJECT_SOURCE_DIR}/sac/tools/lobby/*.cpp
        ${PROJECT_SOURCE_DIR}/sac/tools/lobby/*.h
    )
    add_executable(lobby_server ${lobby_server_source_files})
    target_link_libraries(lobby_server sac z) #WARNING!


    #texture packer
    file(
        GLOB_RECURSE texture_packer_source_files
        ${PROJECT_SOURCE_DIR}/sac/tools/texture_packer/*.cpp
        ${PROJECT_SOURCE_DIR}/sac/tools/texture_packer/*.h
    )
    add_executable(texture_packer ${texture_packer_source_files})
    target_link_libraries(texture_packer sac)
endif()

#unit tests, if installed
find_package (UnitTest++)
if (UNITTEST_FOUND)
    include_directories(${UNITTEST_INCLUDE_DIR})
    file(
            GLOB_RECURSE test_source_files
            ${PROJECT_SOURCE_DIR}/sac/tests/*.cpp
            ${PROJECT_SOURCE_DIR}/tests/*.cpp
    )

    add_test(sac_tests ${test_source_files})

    #the necessary libs
    check_and_link_libs("sac_tests" UnitTest++)
endif ()
