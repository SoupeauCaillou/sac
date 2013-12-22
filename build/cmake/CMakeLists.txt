#######################################################################
####################Target check & validation##########################
#######################################################################
#list of the supported OS
SET(SUPPORTED_TARGETS linux android emscripten windows darwin)

#if target = NULL then target = current OS
if (NOT TARGET)
    string (TOLOWER "${CMAKE_SYSTEM_NAME}" varplatform)
    string (TOUPPER "${CMAKE_SYSTEM_NAME}" VARPLATFORM)
    message("No target defined (TARGET). Targets available: ${SUPPORTED_TARGETS}. Will use your OS ******${varplatform}******")
else()
    string (TOLOWER "${TARGET}" varplatform)
    string (TOUPPER "${TARGET}" VARPLATFORM)
endif()

LIST(FIND SUPPORTED_TARGETS ${varplatform} target_in_list)
#check that it's supported yet
if (target_in_list EQUAL -1)
    message(SEND_ERROR "${varplatform} not in supported targets: ${SUPPORTED_TARGETS}")
    return()
endif()

message ("Build platform: ${varplatform}")
add_definitions(-DSAC_${VARPLATFORM})

#######################################################################
############Check build target (release/debug)#########################
#######################################################################
if (NOT CMAKE_BUILD_TYPE)
    message("No build target specified (CMAKE_BUILD_TYPE), will use ******DEBUG******.")
    set (BUILD_TARGET "DEBUG")
else()
    string (TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TARGET)
endif()

message("Build target: " ${BUILD_TARGET})


#import platform dependent toolchain
include(${PROJECT_SOURCE_DIR}/sac/build/cmake/toolchains/${varplatform}.cmake)

#import gcc/g++ flags relative to toolchain
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS_${BUILD_TARGET}}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS_${BUILD_TARGET}}")

#add flag SAC_DEBUG or SAC_RELEASE
add_definitions(-DSAC_${BUILD_TARGET})

if (SAC_CUSTOM_DEFINES)
    message("Adding the following defines: ${SAC_CUSTOM_DEFINES}")
    add_definitions(${SAC_CUSTOM_DEFINES})
endif()

#set a variable used for the window title
#retrieve git latest tag
execute_process(
        COMMAND git describe --abbrev=0 COMMAND tr -d '\n'
        OUTPUT_VARIABLE SAC_REVISION_TAG
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
#and git latest commit id
execute_process(
        COMMAND git rev-parse HEAD COMMAND head -c 8
        OUTPUT_VARIABLE SAC_REVISION
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
if (SAC_REVISION_TAG OR SAC_REVISION)
    if (SAC_REVISION_TAG)
        set ( SAC_REVISION_TAG "${SAC_REVISION_TAG} - " )
    endif()
    set (SAC_REVISION_TAG "${SAC_REVISION_TAG}${SAC_REVISION}")
    string(STRIP ${SAC_REVISION_TAG} SAC_REVISION_TAG)

    message("Revision tag: '${SAC_REVISION_TAG}'")
    add_definitions(-DSAC_REVISION_TAG="${SAC_REVISION_TAG}")
endif()


#######################################################################
########Library path finding related tools#############################
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

include(cotire)
#######################################################################
##########Build headers files for .vs and .fs files####################
#######################################################################

# 1st step: add a recipe to build a .h for each [vf]s files
file (
    GLOB_RECURSE
    SHADER_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/sac/systems/opengl/shaders/*.[vf]s
)

foreach (FILE ${SHADER_SOURCE_FILES})
    file(RELATIVE_PATH
        REL_FILE
        ${PROJECT_SOURCE_DIR}/sac/systems/opengl/shaders
        ${FILE})

    #output format: foo.vs -> foo_vs.h
    STRING(REGEX REPLACE "\\." "_" Output ${FILE} )
    message ("Add rule to build shader: ${REL_FILE} -> ${Output}.h")

    SET_SOURCE_FILES_PROPERTIES(${Output}.h
        PROPERTIES GENERATED TRUE)

    add_custom_command(
        OUTPUT ${Output}.h
        COMMAND xxd -i ${REL_FILE} | sed s/}/,0x00}/ > ${Output}.h
        DEPENDS ${FILE}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/sac/systems/opengl/shaders
        COMMENT "Building shader ${REL_FILE}"
    )
endforeach()

# 2nd step: define 'generate_shaders' target and make it depends on .h file
string(REGEX REPLACE "\\.([vf]s)" "_\\1.h;" SHADER_HEADER_FILES ${SHADER_SOURCE_FILES})
add_custom_target(generate_shaders DEPENDS ${SHADER_HEADER_FILES})

# 3rd step (see below): make sac target depends on generate_shaders target

#######################################################################
##################Make assets (from unprepared_assets##################
#######################################################################

#file(
#    GLOB sub-dir
#    ${PROJECT_SOURCE_DIR}/unprepared_assets/*)

#message ("${sub-dir}")

#foreach (dir ${sub-dir})
#    if (IS_DIRECTORY ${dir})
#        message("Add rules to make assets")
#        message("Dir found : -> ${dir}")

#        add_custom_command(
#            COMMAND ${PROJECT_SOURCE_DIR}/sac/tools/generate_altas.sh
#            DEPENDS ${dir}
#            WORKING_DIRECTORY ${dir})
#    endif()
#endforeach()


#######################################################################
######################How to build sac#################################
#######################################################################

if (DEFINED extra_source_directories)
    include_directories(${extra_source_directories})
    file(
        GLOB_RECURSE extra_source_files
        ${extra_source_directories}/*
    )
endif()

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
    ${PROJECT_SOURCE_DIR}/sac/api/default/*
)

if (WEB_BUILD)
    set(EXECUTABLE_NAME ${PROJECT_NAME}.bc)
else()
    set(EXECUTABLE_NAME ${PROJECT_NAME})

    file (
        GLOB_RECURSE opt_source_files
        ${PROJECT_SOURCE_DIR}/sac/libs/libpng/jni/*.c #libpng
    )
endif()

#include platform dependent APIs
if (DESKTOP_BUILD OR WEB_BUILD)
    file(
        GLOB_RECURSE platform_source_files
        ${PROJECT_SOURCE_DIR}/sac/app/*
        ${PROJECT_SOURCE_DIR}/sac/api/linux/*
        ${PROJECT_SOURCE_DIR}/platforms/default/api/*
    )
elseif (MOBILE_BUILD)
    file(
        GLOB_RECURSE platform_source_files
        ${PROJECT_SOURCE_DIR}/sac/api/android/*
        ${PROJECT_SOURCE_DIR}/sources/api/android/*
        ${PROJECT_SOURCE_DIR}/sac/android/*
        ${PROJECT_SOURCE_DIR}/platforms/android/*.cpp
    )
else()
    message(SEND_ERROR "which platform?")
endif()

add_library(sac ${SAC_LIB_TYPE} ${source_files} ${opt_source_files} ${platform_source_files} ${extra_source_files})
add_dependencies( sac generate_shaders ${extra_custom_target})

#######################################################################
##########Build and link libraries per target##########################
#######################################################################
if (NETWORK_BUILD)
    ########## build enet lib ###########
    # following defs replace enet's config.h
    add_definitions(-DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1
    -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1
    -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1
    -DHAS_POLL=1 -DHAS_FCNTL=1 -DHAS_INET_PTON=1 -DHAS_INET_NTOP=1 -DHAS_MSGHDR_FLAGS=1
    -DHAS_SOCKLEN_T=1 -DSAC_NETWORK=1)

    set (enet_base ${PROJECT_SOURCE_DIR}/sac/libs/enet)
    file(
        GLOB_RECURSE enet_sources
        ${enet_base}/*.c
    )
    include_directories(${PROJECT_SOURCE_DIR}/sac/libs/enet/include)

    add_library("enet" STATIC ${enet_sources})

    target_link_libraries("sac" enet)
endif()

if (DESKTOP_BUILD)
    check_and_link_libs("sac" AntTweakBar GLEW Ogg OpenAL SDL Sqlite3 Tinyxml2 VorbisFile Vorbis ZLIB)
endif()

if (MOBILE_BUILD OR WEB_BUILD)
    if (MOBILE_BUILD)
        ########## build sqlite3 lib ###########
        set (sqlite3_base ${PROJECT_SOURCE_DIR}/sac/libs/sqlite3)
        file(
            GLOB_RECURSE sqlite3_sources
            ${sqlite3_base}/sqlite3.c
        )

        include_directories(${PROJECT_SOURCE_DIR}/sac/libs/sqlite3)
        add_library("sqlite3" STATIC ${sqlite3_sources})

        target_link_libraries("sac" GLESv2 z ogg log sqlite3)
    else ()
        target_link_libraries("sac" GLESv2 z ogg log)
    endif()
endif()

if(MOBILE_BUILD)
   ########## build ogg lib ###########
   set (ogg_base ${PROJECT_SOURCE_DIR}/sac/libs/tremor)

    if (NOT ANDROID_ABI STREQUAL x86)
        enable_language(ASM)
        file(
            GLOB_RECURSE ogg_sources
            ${ogg_base}/*.s
            ${ogg_base}/*.c
        )
        add_library("ogg" STATIC ${ogg_sources})
        set_target_properties("ogg" PROPERTIES COMPILE_FLAGS
            "-DLITTLE_ENDIAN=1 -DBIG_ENDIAN=2 -DBYTE_ORDER=LITTLE_ENDIAN -D_ARM_ASSEM_")
    else()
        file(
            GLOB_RECURSE ogg_sources
            ${ogg_base}/*.c
        )
        add_library("ogg" STATIC ${ogg_sources})
        set_target_properties("ogg" PROPERTIES COMPILE_FLAGS
            "-DLITTLE_ENDIAN=1 -DBYTE_ORDER=LITTLE_ENDIAN")
    endif()

    ##########do the linkage with sac ###########
    target_link_libraries("sac" GLESv2 z ogg log)
endif()

#######################################################################
#######################Import OS-dependent libs########################
#######################################################################
import_specific_libs()

#######################################################################
#######Build the game executable (except for mobile)###################
#######################################################################
if (NOT MOBILE_BUILD)

    file(
        GLOB source_platforms_files
        ${PROJECT_SOURCE_DIR}/platforms/default/*.cpp
    )
    add_executable(${EXECUTABLE_NAME} ${source_platforms_files})
    target_link_libraries(${EXECUTABLE_NAME} sac)
    cotire(${EXECUTABLE_NAME})
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

if (NETWORK_BUILD)
    ########## lobby server ###########
    file(
        GLOB_RECURSE lobby_server_source_files
        ${PROJECT_SOURCE_DIR}/sac/tools/lobby/*.cpp
        ${PROJECT_SOURCE_DIR}/sac/tools/lobby/*.h
    )
    add_executable(lobby_server ${lobby_server_source_files})
    target_link_libraries(lobby_server sac)
endif()

function (generate_sublime_clang_configuration)
    #retrieve all our defines
    get_directory_property(defs COMPILE_DEFINITIONS)
    list(LENGTH defs len)
    math(EXPR len ${len}-1)
    set (DEFINES "")
    #and for each, add them to our custom variables DEFINES, adding -D before 
    #them and comma separated
    foreach (item RANGE ${len})
        list(GET defs ${item} value)

        #.... because this will be placed in another .cmake file so we must 
        #meta-escape quotes
        string(REPLACE "\"" "\\\\\\\"" value ${value})

        #finally add the item to our string list
        set (DEFINES "${DEFINES} '-D${value}',\n")
    endforeach()

    #this is the content of the created .cmake file
    set(SUBLIME_CLANG_PARAMETERS 
        "set (SUBLIME_CLANG_PARAMETERS \",
                'settings':
                {
                    'sublimeclang_additional_language_options':
                    {
                        'c++' : ['-std=c++11']
                    },

                    'sublimeclang_options':
                    [
                        '-ferror-limit=0',

                        '-I/usr/include/x86_64-linux-gnu/c++/4.8/',
                        '-I/usr/include/unittest++/',
                        '-I/usr/include/AL/',

                        ${DEFINES}

                        '-I\${project_path}/**'
                    ]
                }
            #end of file '{'
            }
        \")
        message(\"post build planished\")

        #replace simple quotes with double quotes
        string(REPLACE \"'\" \"\\\"\" SUBLIME_CLANG_PARAMETERS \${SUBLIME_CLANG_PARAMETERS})
        if (EXISTS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sublime-project)
            #remove last '{' from current project since we need to add some content
            execute_process(COMMAND sed -i \"$ d\" ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sublime-project)
            #finally add ^ SUBLIME_CLANG_PARAMETERS to the sublime-project file
            file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sublime-project \${SUBLIME_CLANG_PARAMETERS})
        endif()
    "
    )

    #write the upon defined variable in a custom cmake file
    #we do NOT execute this directly (would be too easy :-)) because we need to wait 
    #POST_BUILD step since the sublime-project file is generated during it
    file(WRITE ${CMAKE_BINARY_DIR}/sublime_clang_plugin_support.cmake ${SUBLIME_CLANG_PARAMETERS})
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} POST_BUILD
        #we execute the generated cmake file at post build!
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/sublime_clang_plugin_support.cmake
    )
endfunction()

generate_sublime_clang_configuration()
