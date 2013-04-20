#!/bin/bash

#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
cd $whereAmI
#if we executed a linked script; go to the real one
if [  -h $0 ]; then
	whereAmI+=/$(dirname $(readlink $0))
	cd $whereAmI
fi

#import cool stuff
source ../coolStuff.sh

#how to use the script
export USAGE="$0 [scripts_options] -[specifics_options] [and their values]"
export OPTIONS="n: simply compile
\tC: remove all cache files (rm -r rm CMakeCache.txt CMakeFiles cmake_install.cmake linux Makefile sac sources 2>/dev/null)
\tR: reset the terminal screen before compiling
\tr: run the app\t\t\t\t\t(options available, see below)
\td: run&debug the app with cgdb
\tl: use colorlog.sh script for colored logs\t(options available, see below)
You can also specify arguments:
\t-c|--cmakeconfig \"arguments for cmake\": see cmake for options.
\t-h|--help: show this help
\t-l|--log \"arguments for coloredlog script\": options for this script. See it for arguments availables
\t-r|--run \"arguments for game\": arguments handled by the game (--restore, --verbose, ..., whatever you did!)"
export EXAMPLE="${green}'$0 RCl -c \"-DCMAKE_BUILD_TYPE=DEBUG\" --run \"--restore\"'${yellow} will clean the screen, compile the game for DEBUG target, then launch the game with restore"

######### 0 : Check requirements. #########
	if [ -z "$(pwd | grep /sac/tools/build)" ]; then
		error_and_quit "The script must be in sac/tools/build !"
	fi

######### 1 : Read arguments. #########
	if [ $# = 0 ]; then
		usage_and_quit
    fi

    CMAKE_CONFIG=""
    TARGETS=""
    RUN_ARGS=""
    COLOREDLOGS_ARGS=""
    while [ "$1" != "" ]; do
        case $1 in
            "-c" | "--cmakeconfig")
                shift
                CMAKE_CONFIG=$1
                ;;
            "-h" | "--help")
                usage_and_quit
                ;;
            "-l" | "--log")
                shift
                COLOREDLOGS_ARGS=$1
                ;;
            "-r" | "--run")
                shift
                RUN_ARGS=$1
                ;;
            -*)
                echo "unknown option: " $1
                ;;
            *)
                TARGETS=$TARGETS$1
        esac
        shift
    done


######### 2 : Create build dir. #########
	rootPath=$whereAmI"/../../.."

	mkdir -p $rootPath/build/linux

######### 3 : Go into build/emscripten #########
	cd $rootPath/build/linux

######### 4 : Execute query. #########
	gameName=$(cat $rootPath/CMakeLists.txt | grep 'project(' | cut -d '(' -f2 | tr -d ')')

	if [ ! -z $(echo $TARGETS | grep R) ]; then
		reset
	fi

	if [ ! -z $(echo $TARGETS | grep C) ]; then
		info "Cleaning.."
		rm -r CMakeCache.txt CMakeFiles cmake_install.cmake linux Makefile sac sources 2>/dev/null
	fi

	if [ ! -z $(echo $TARGETS | grep -e n -e r -e d) ]; then
		info "Compiling.."
		if (!(cmake $rootPath $CMAKE_CONFIG)); then
			error_and_quit "Error in cmake. Maybe should run with C option?"
		elif (!(make -j4)); then
			error_and_quit "Error in make"
		fi
	fi

	executable=./$gameName $RUN_ARGS
	#debug required
	if [ ! -z $(echo $TARGETS | grep d) ]; then
		info "A bug? Cgdb on the way!"
        #(echo r; cat) | gdb $executable
        cgdb $executable
	#launch required
	elif [ ! -z $(echo $TARGETS | grep r) ]; then
		#verbose required
		if [ ! -z $(echo $TARGETS | grep l) ]; then
			info "Launch with colored log."

            #coloredlogs shouldn't be empty
            if [ -z "$COLOREDLOGS_ARGS" ]; then
                info "No arg for color script ?\nChoose tag with option -l" $red
                info "I will use 'all' tag here"
                COLOREDLOGS_ARGS='all'
                sleep 3
            fi
			$executable | $rootPath/sac/tools/build/linux-coloredLogs.sh $COLOREDLOGS_ARGS

		else
			info "Launch game."
			$executable
		fi
	fi






