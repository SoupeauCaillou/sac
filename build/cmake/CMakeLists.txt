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
##########Build headers files for .vs and .fs files####################
#######################################################################
execute_process(COMMAND ls
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/sac/systems/opengl/shaders
    OUTPUT_VARIABLE FILES)
#set it into a list
string(REPLACE "\n" ";" FILES_LIST ${FILES})
foreach (FILE ${FILES_LIST})
    if (NOT ${FILE} MATCHES "(.*).h")
        #output format: foo.vs -> foo_vs.h
        STRING(REGEX REPLACE "\\." "_" Output ${FILE} )

        execute_process(COMMAND xxd -i ${FILE} ${Output}.h
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/sac/systems/opengl/shaders)
    endif()
endforeach()

#######################################################################
######################How to build SAC#################################
#######################################################################
include_directories(${PROJECT_SOURCE_DIR}/sources)
include_directories(${PROJECT_SOURCE_DIR}/sac)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/libpng/jni)
include_directories(${PROJECT_SOURCE_DIR}/sac/libs/glm)

add_definitions(-DGLM_FORCE_RADIANS)

file(
    GLOB_RECURSE source_files
    ${PROJECT_SOURCE_DIR}/sac/base/*
    ${PROJECT_SOURCE_DIR}/sac/systems/*
    ${PROJECT_SOURCE_DIR}/sac/util/*
    ${PROJECT_SOURCE_DIR}/sac/steering/*
    ${PROJECT_SOURCE_DIR}/sources/*
)

if (WEB_BUILD)
    set(EXECUTABLE_NAME ${PROJECT_NAME}.bc)

    add_library(sac ${LIB_TYPE} ${source_files})
    check_and_link_libs("sac" SDL)
else()
    set(EXECUTABLE_NAME ${PROJECT_NAME})

    file(
        GLOB_RECURSE opt_source_files
        ${PROJECT_SOURCE_DIR}/sac/libs/libpng/jni/*.c #libpng
        ${PROJECT_SOURCE_DIR}/sac/libs/jsoncpp-src-0.6.0-rc2/src/lib_json/*.cpp #lib json
        ${PROJECT_SOURCE_DIR}/sac/libs/enet/*.c #lib enet
    )

    # following defs replace enet's config.h
    add_definitions(-DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1
    -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1
    -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1
    -DHAS_POLL=1 -DHAS_FCNTL=1 -DHAS_INET_PTON=1 -DHAS_INET_NTOP=1 -DHAS_MSGHDR_FLAGS=1
    -DHAS_SOCKLEN_T=1)

    include_directories(${PROJECT_SOURCE_DIR}/sac/libs/enet/include)
    include_directories(${PROJECT_SOURCE_DIR}/sac/libs/jsoncpp-src-0.6.0-rc2/include)


    #include platform dependent APIs
    if (DESKTOP_BUILD)
        file(
            GLOB_RECURSE platform_source_files
            ${PROJECT_SOURCE_DIR}/sac/app/*
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

    add_library(sac ${SAC_LIB_TYPE} ${source_files} ${opt_source_files} ${platform_source_files})
    #the necessary libs for not emscripten
    if (DESKTOP_BUILD)
        check_and_link_libs("sac" GLEW GLFW )
    endif()
endif()

#######################################################################
######################Libraries for all targets########################
#######################################################################
if (DESKTOP_BUILD)
    check_and_link_libs("sac" OpenAL Sqlite3 VorbisFile Vorbis ZLIB
        AntTweakBar Ogg Tinyxml2)
endif()

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
    add_executable(${EXECUTABLE_NAME} ${source_platforms_files})
    target_link_libraries(${EXECUTABLE_NAME} sac)
elseif (MOBILE_BUILD)
    ENABLE_LANGUAGE(ASM)
    # build ogg lib
    set (ogg_base ${PROJECT_SOURCE_DIR}/sac/libs/tremor)
    set (ogg_sources ${ogg_base}/bitwiseARM.s
        ${ogg_base}/bitwise.c
        ${ogg_base}/codebook.c
        ${ogg_base}/dpenARM.s
        ${ogg_base}/dsp.c
        ${ogg_base}/floor0.c
        ${ogg_base}/floor1ARM.s
        ${ogg_base}/floor1.c
        ${ogg_base}/floor1LARM.s
        ${ogg_base}/floor_lookup.c
        ${ogg_base}/framing.c
        ${ogg_base}/info.c
        ${ogg_base}/mapping0.c
        ${ogg_base}/mdctARM.s
        ${ogg_base}/mdct.c
        ${ogg_base}/mdctLARM.s
        ${ogg_base}/misc.c
        ${ogg_base}/res012.c
        ${ogg_base}/vorbisfile.c)
    add_library("ogg" STATIC ${ogg_sources})
    set_target_properties("ogg" PROPERTIES COMPILE_FLAGS
        "-DLITTLE_ENDIAN=1 -DBIG_ENDIAN=2 -DBYTE_ORDER=LITTLE_ENDIAN -D_ARM_ASSEM_")

    file(
        GLOB_RECURSE source_platforms_files
    )
    target_link_libraries("sac" GLESv2 z ogg)
elseif (WEB_BUILD)
    #for emscripten, all the files are in sac
    set(source_platforms_files "")
else()
    message(SEND_ERROR "which platform?")
endif()

#######################################################################
#############################Post build actions########################
#######################################################################
postbuild_specific_actions()

#######################################################################
#############################others executables########################
#######################################################################
others_specific_executables()

if (DESKTOP_BUILD)
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
endif()
