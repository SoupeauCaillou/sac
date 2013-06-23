#!/bin/bash

###################################################
###################################################
###################################################
echo "[1m[31mSTILL TO DO:\
- refaire le script android
- mode interactif
[0m
"
###################################################
###################################################
###################################################

#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
cd $whereAmI
#if we executed a linked script; go to the real one
if [  -h $0 ]; then
	whereAmI+=/$(dirname $(readlink $0))
	cd $whereAmI
fi
rootPath=$whereAmI"/../../.."

#import cool stuff
source ../coolStuff.sh

#get the list of available targets (need to be displayed in OPTIONS help
TARGETS_LIST=$(grep -m 1 SUPPORTED_TARGETS ../../build/cmake/CMakeLists.txt | cut -d " " -f 2- | tr -d ')')

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
\t--t|--target target_name: specify the target name (in the following list: '$TARGETS_LIST')
\t--c|--cmakeconfig \"arguments for cmake\": see cmake for options."


export SAC_EXAMPLE="${green}TODO${default_color}"

######### 0 : Check requirements. #########
	if [ -z "$(pwd | grep /sac/tools/build)" ]; then
		error_and_quit "The script must be in sac/tools/build !"
	fi

######### 1 : Read arguments. #########
    CMAKE_CONFIG=""
    TARGETS=""
    RUN_ARGS=""
    COLOREDLOGS_ARGS=""
    CMAKE_BUILD_TARGET=""
    SHOULD_QUIT_AFTER_READ=0
    ARGS=$@
    while [ "$1" != "" ]; do
        case $1 in
            "-h" | "-help")
                SHOULD_QUIT_AFTER_READ=1
                ;;
            "--c" | "--cmakeconfig")
                shift
                TARGETS=$TARGETS"n"
                CMAKE_CONFIG=$CMAKE_CONFIG" $1"
                ;;
            "-release")
                CMAKE_CONFIG=$CMAKE_CONFIG" -DCMAKE_BUILD_TYPE=release"
                ;;
            "-debug")
                CMAKE_CONFIG=$CMAKE_CONFIG" -DCMAKE_BUILD_TYPE=debug"
                ;;
             "--t" | "--target")
                shift
                CMAKE_BUILD_TARGET=$1
                ;;
            --*)
                shift
                ;;
            -*)
                ;;
            *)
                TARGETS=$TARGETS$1
        esac
        shift

    done

    #if we didn't ask for help, get the target config
    if [ $SHOULD_QUIT_AFTER_READ = 0 ]; then
        # if build target is empty, ask user for it
        if [ -z "$CMAKE_BUILD_TARGET" ]; then
            info "You haven't choosen any target with --target option. Please select one in the list:"
            select CMAKE_BUILD_TARGET in $TARGETS_LIST
            do
                echo $result
                break
            done
        fi
        #convert to lowercase
        CMAKE_BUILD_TARGET=$(echo $CMAKE_BUILD_TARGET | tr '[:upper:]' '[:lower:]')
        #save for cmake
        CMAKE_CONFIG=$CMAKE_CONFIG" -DTARGET=$CMAKE_BUILD_TARGET"
        #import the script
        source build-$CMAKE_BUILD_TARGET.sh

        #let him parse the args if necessary
        parse_arguments $ARGS
    fi

    #if there is no target or help was asked
    if [ -z "$TARGETS" ] || [ $SHOULD_QUIT_AFTER_READ = 1 ]; then
        if [ ! -z "$CMAKE_BUILD_TARGET" ]; then
            source build-$CMAKE_BUILD_TARGET.sh
            SAC_OPTIONS=$SAC_OPTIONS"\nFor $CMAKE_BUILD_TARGET platform, you have the following options:\n\
            $PLATFORM_OPTIONS"
        fi
        usage_and_quit
    fi


    # retrieve build type
    CMAKE_BUILD_TYPE="debug"
    if [ ! -z "$(grep -i -- '-DCMAKE_BUILD_TYPE=' <<< "$CMAKE_CONFIG")" ]; then
        CMAKE_BUILD_TYPE=$(echo $CMAKE_CONFIG | sed 's/-DCMAKE_BUILD_TYPE=/~/' | cut -d '~' -f2 | cut -d ' ' -f1)
    fi

######### 2 : Create build dir. #########
    builddir=$rootPath/build/$CMAKE_BUILD_TARGET-$CMAKE_BUILD_TYPE
	mkdir -p $builddir

######### 3 : Go into build directory #########
	cd $builddir

######### 4 : Execute query. #########
	gameName=$(cat $rootPath/CMakeLists.txt | grep 'project(' | cut -d '(' -f2 | tr -d ')')

#Cleaning
	if [ ! -z "$(echo $TARGETS | grep R)" ]; then
		reset
	fi

	if [ ! -z "$(echo $TARGETS | grep C)" ]; then
        info "Are you sure you want to clean current build('build/$CMAKE_BUILD_TARGET-$CMAKE_BUILD_TYPE') directory? Press enter to confirm..." $yellow
        read aaa${RANDOM}ndomav
		info "Cleaning.."
		rm -r CMakeCache.txt CMakeFiles cmake_install.cmake linux Makefile sac sources 2>/dev/null
	fi

#Compiling
	if [ ! -z "$(echo $TARGETS | grep -e n)" ]; then
        info "Compiling.."

        compilation_before

        if (!(cmake $CMAKE_CONFIG $rootPath)); then
            compilation_after
            error_and_quit "Error in cmake. Maybe should run with C option?"
        fi

        if (!(make -j4)); then
            compilation_after
            error_and_quit "Error in make"
        fi

        compilation_after
	fi


#Launching
    launch_the_application $TARGETS $RUN_ARGS





