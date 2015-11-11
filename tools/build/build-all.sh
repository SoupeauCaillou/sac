#!/bin/bash

#from where are we calling it
fromWhereAmIBeingCalled=$PWD
#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
cd $whereAmI
#if we executed a linked script; go to the real one
if [  -h $0 ]; then
    whereAmI+=/$(dirname $(readlink $0))
    cd $whereAmI
fi
rootPath=$whereAmI"/../../.."
gameName=$(grep 'project(' $rootPath/CMakeLists.txt | cut -d '(' -f2 | tr -d ')')

#import cool stuff
source ../cool_stuff.sh

#get the list of available targets (need to be displayed in OPTIONS help
targets_list=$(grep SUPPORTED_TARGETS $rootPath/sac/CMakeLists.txt | head -1 | cut -d " " -f 2- | tr -d ')')
build_system_list="ninja makefile"
#how to use the script
export SAC_USAGE="$0 [options]"
export SAC_OPTIONS="\
n: simply compile
\tC: remove all cache files (rm -r rm CMakeCache.txt CMakeFiles cmake_install.cmake linux Makefile sac sources 2>/dev/null)
\tR: reset the terminal screen before compiling
\tr: start the app
You can also specify arguments:
\t-h|-help: show this help
\t-release|-debug: build type specifying
\t--t|--target target_name: specify the target name (in the following list: '$targets_list')
\t--g|--generate ${build_system_list// /|}: generate build system files and sublime project
\t--c|--cmakeconfig \"arguments for cmake\": see cmake for options."
export SAC_EXAMPLE="${green}TODO${default_color}"

######### 0 : Check requirements. #########
    if [ -z "$(pwd | grep /sac/tools/build)" ]; then
        error_and_quit "The script must be in sac/tools/build !"
    fi

######### 1 : Read arguments. #########
    cmake_config=()
    targets=""
    run_args=""
    coloredlogs_args=""
    cmake_build_target=""
    should_quit_after_read=0
    build_system=""
    args="" #DONT DID args=$@ nor args="$@" because it'll mess up variables containing spaces
    while [ "$1" != "" ]; do
        case $1 in
            "-h" | "-help")
                should_quit_after_read=1
                ;;
            "-release")
                cmake_config+=(-DCMAKE_BUILD_TYPE=release)
                ;;
            "-debug")
                cmake_config+=(-DCMAKE_BUILD_TYPE=debug)
                ;;
             "--t" | "--target")
                shift
                cmake_build_target=$1
                ;;
            "--c" | "--cmakeconfig")
                shift
                targets=$targets"n"
                cmake_config+=($1)
                ;;
            "--g")
                shift
                targets=$targets"n"
                build_system=$1
                ;;
            --*)
                args+=" "$1
                shift
                args+=" "$1
                ;;
            -*)
                args+=" "$1
                ;;
            *)
                targets=$targets$1
        esac
        shift
    done
    #if we didn't ask for help, get the target config
    if [ $should_quit_after_read = 0 ]; then
        # if build target is empty, ask user for it
        if [ -z "$cmake_build_target" ]; then
            info "You haven't choosen any target with --target option. Please select one in the list:"
            select cmake_build_target in $targets_list
            do
                echo $result
                break
            done
        fi
        #convert to lowercase
        cmake_build_target=$(echo $cmake_build_target | tr '[:upper:]' '[:lower:]')
        #save for cmake
        cmake_config+=(-DTARGET=$cmake_build_target)
        #import the script
        source build-$cmake_build_target.sh

        #let it parse the args if necessary
        if ! parse_arguments $args ; then
            error_and_quit "Some arguments are incorrect"
        fi
    fi

    #if there is no target or help was asked
    if [ -z "$targets" ] || [ $should_quit_after_read = 1 ]; then
        if [ ! -z "$cmake_build_target" ]; then
            source build-$cmake_build_target.sh
            SAC_OPTIONS=$SAC_OPTIONS"\nFor $cmake_build_target platform, you have the following options:\n$PLATFORM_OPTIONS"
        fi
        usage_and_quit
    fi


    # retrieve build type
    cmake_build_type="debug"
    if [ ! -z "$(grep -i -- '-DCMAKE_BUILD_TYPE=' <<< "${cmake_config[@]}")" ]; then
        cmake_build_type=$(echo ${cmake_config[@]} | sed 's/-DCMAKE_BUILD_TYPE=/~/' | cut -d '~' -f2 | cut -d ' ' -f1)
    fi

    builddir=$rootPath/build/$cmake_build_target-$cmake_build_type

######### 2 : Verify needed softwares are installed. #########
    check_necessary

######### 3 : Create build dir and go inside #########
    mkdir -p $builddir
    cd $builddir

######### 4 : Execute query. #########

#Cleaning
    if [ ! -z "$(echo $targets | grep R)" ]; then
        reset
    fi

    if [ ! -z "$(echo $targets | grep C)" ]; then
        info "Are you sure you want to clean current build('$builddir') directory? Press enter to confirm..." $yellow
        read aaa${RANDOM}ndomav
        info "Cleaning.."
        rm -r * &>/dev/null
    fi

    init

#Compiling
    if [ ! -z "$(echo $targets | grep -e n)" ]; then
        info "Compiling.."

        check_package cmake

        if [ ! -z "$build_system" ]; then
            real_name=""
            if ! [[ " $build_system_list " =~ " $build_system " ]]; then
                error_and_quit "Unknown build system '$build_system'."
            elif [ "$build_system" = "ninja" ]; then
                check_package ninja ninja-build
                real_name="Ninja"
            elif [ "$build_system" = "makefile" ]; then
                real_name="Unix Makefiles"
            fi
            cmake_config+=(-G "Sublime Text 2 - $real_name")
        fi

        compilation_before
        if ! cmake "${cmake_config[@]}" $rootPath; then
            compilation_after

            error_and_quit "Error in cmake:
            - If this is the first time you run cmake, please view $(readlink -f ${rootPath}/sac/INSTALL).
            - Otherwise, your cmake configuration might be wrong, considere cleaning it (add parameter 'C' to $0)?\n"
        fi

        if [ -f Makefile ]; then
            build_command="make"
        elif [ -f build.ninja ]; then
            build_command='ninja'
        else
            error_and_quit 'No makefile/build.ninja found in $PWD'
        fi

        check_package $build_command
        if ! $build_command; then
            compilation_after
            error_and_quit "Error when building with '$build_command'"
        fi

        compilation_after
    fi


#Launching
    launch_the_application $targets $run_args

info "Good bye, my Lord!"
