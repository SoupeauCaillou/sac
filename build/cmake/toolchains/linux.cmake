ADD_DEFINITIONS(-DSAC_DESKTOP=1 -DSAC_ENABLE_LOG=1 -DSAC_INGAME_EDITORS=1
    -DSAC_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/" -DSAC_ENABLE_PROFILING=1)
    # -DSAC_USE_VBO=1)

# Enable '-D_GLIBCXX_DEBUG' to debug stl containers related issues

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread -O0")
set(CXX_FLAGS_DEBUG "-Wall -W -g")

# if (${CMAKE_C_COMPILER} MATCHES "(.*)clang")
    # workaround bug http://llvm.org/bugs/show_bug.cgi?id=12730
    # message ("clang compiler detected -> workaround bug #12730")
    # add_definitions(-D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)
# endif()

set(DESKTOP_BUILD 1)

SET (SAC_LIB_TYPE STATIC)

function (others_specific_executables)
    #texture packer
    file(
        GLOB_RECURSE texture_packer_source_files
        ${PROJECT_SOURCE_DIR}/sac/tools/texture_packer/*.cpp
        ${PROJECT_SOURCE_DIR}/sac/tools/texture_packer/*.h
    )
    add_executable(texture_packer ${texture_packer_source_files})
    target_link_libraries(texture_packer sac rt)
endfunction()

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
        #replace simple quotes with double quotes
        string(REPLACE \"'\" \"\\\"\" SUBLIME_CLANG_PARAMETERS \${SUBLIME_CLANG_PARAMETERS})
        #remove last '{' from current project since we need to add some content
        execute_process(COMMAND sed -i \"$ d\" ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sublime-project)
        #finally add ^ SUBLIME_CLANG_PARAMETERS to the sublime-project file
        file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sublime-project \${SUBLIME_CLANG_PARAMETERS})
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

function (postbuild_specific_actions)
    target_link_libraries(${EXECUTABLE_NAME} GL)

    generate_sublime_clang_configuration()
endfunction()

function (import_specific_libs)
    check_and_link_libs("sac" VPX GL CURL)
endfunction()
