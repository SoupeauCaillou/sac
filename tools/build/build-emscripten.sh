#!/bin/bash

#how to use the script
export USAGE="$0 [options]"
export OPTIONS="n: simply compile
\tC: remove all cache files (rm -r rm CMakeCache.txt CMakeFiles cmake_install.cmake linux Makefile sac sources 2>/dev/null)
\tr: reset the terminal screen before compiling"
export EXAMPLE="'$0 n-C' will clean cached files then compile."

#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
cd $whereAmI
#if we executed a linked script; go to the real one
if [  -h $0 ]; then
	whereAmI=$(dirname $(readlink $0))
	cd $whereAmI
fi

#import cool stuff
source ../coolStuff.sh

######### 0 : Check requirements. #########
	check_package "emcc"
	if [ -z "$(pwd | grep /sac/tools/build)" ]; then
		error_and_quit "The script must be in sac/tools/build !"
	fi

######### 1 : Check arguments. #########
	if [ $# != 1 ] || [ $(echo $1 | grep -- -h) ]; then
		usage_and_quit
	fi



######### 2 : Create build dir. #########
	rootPath=$whereAmI/../../..

	mkdir -p $rootPath/build/emscripten


######### 3 : Go into build/emscripten #########
	cd $rootPath/build/emscripten

######### 4 : Execute query. #########
	if [ ! -z $(echo $1 | grep r) ]; then
		reset
	fi

	if [ ! -z $(echo $1 | grep C) ]; then
		info "Cleaning.."
		rm -r CMakeCache.txt CMakeFiles 2>/dev/null
	fi

	if [ ! -z $(echo $1 | grep n) ]; then
		info "Compiling.."
		echo $PWD
		echo $rootPath
		if (!(cmake -DCMAKE_TOOLCHAIN_FILE=$rootPath/emscripten.cmake $rootPath)); then
			error_and_quit "Error in cmake"
		elif (!(make -j4)); then
			error_and_quit "Error in make"
		fi
	fi

