#######################################################################
########Library path finding related tools############################
#######################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/sac/build/cmake/Modules")

#ensure that a lib is installed and link it to "target" executable
function (check_and_link_libs target)
   #you shouldn't touch that
    foreach(lib ${ARGN})
        string(TOUPPER ${lib} LIB) #in uppercase

        find_package (${lib})
        if (${LIB}_FOUND)
          include_directories(${${LIB}_INCLUDE_DIR})
          target_link_libraries (${target} ${${LIB}_LIBRARY})
        else()
            message(SEND_ERROR "You NEED ${lib} library.")
        endif ()
    endforeach()
endfunction()


#######################################################################
######################How to build SAC#################################
#######################################################################
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

if (WEB_BUILD)
    set(EXECUTABLE_NAME ${PROJECT_NAME}.bc)

    add_library(sac ${source_files})
    check_and_link_libs("sac" SDL)
else()
    set(EXECUTABLE_NAME ${PROJECT_NAME})

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
        message(SEND_ERROR "which platform?")
    endif()

    add_library(sac ${source_files} ${opt_source_files} ${platform_source_files})
    #the necessary libs for not emscripten
    check_and_link_libs("sac" GLEW GLFW )
endif()

#######################################################################
######################Libraries for all targets########################
#######################################################################
check_and_link_libs("sac" OpenAL Sqlite3 VorbisFile Vorbis ZLIB
AntTweakBar Ogg Tinyxml2)

#######################################################################
#######################Import OS-dependent libs########################
#######################################################################
import_specific_libs()

#######################################################################
#######################Build the game executable#######################
#######################################################################
if (DESKTOP_BUILD)
    file(
        GLOB_RECURSE source_platforms_files
        ${PROJECT_SOURCE_DIR}/platforms/default/*
    )
elseif (MOBILE_BUILD)
    file(
        GLOB_RECURSE platform_source_files
        ${PROJECT_SOURCE_DIR}/sac/api/android/*
        ${PROJECT_SOURCE_DIR}/sources/api/android/*
    )
elseif (WEB_BUILD)
    #for emscripten, all the files are in sac
    set(source_platforms_files "")
else()
    message(SEND_ERROR "which platform?")
endif()

add_executable(${EXECUTABLE_NAME} ${source_platforms_files})
target_link_libraries(${EXECUTABLE_NAME} sac)

#######################################################################
#############################Post build actions########################
#######################################################################
postbuild_specific_actions()

#######################################################################
#############################others executables########################
#######################################################################
others_specific_executables()

#unit tests, if installed
find_package (UnitTest++)
if (UNITTEST_FOUND)
    file(
            GLOB_RECURSE test_source_files
            ${PROJECT_SOURCE_DIR}/sac/tests/*.cpp
            ${PROJECT_SOURCE_DIR}/tests/*.cpp
    )
    add_executable(sac_tests ${test_source_files})

    #the necessary libs
    include_directories(${UNITTEST_INCLUDE_DIR})
    target_link_libraries ("sac_tests" ${UNITTEST_LIBRARY})

    target_link_libraries("sac_tests" sac)

    #launch sac_tests after each build
    add_custom_command(TARGET sac_tests POST_BUILD COMMAND sac_tests)
endif ()
