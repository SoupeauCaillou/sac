#!/bin/bash

#how to use the script
export USAGE="$0"

#where the script is
whereAmI=`cd "$(dirname "$0")" && pwd`
#import cool stuff
if [ ! -z "$(echo $whereAmI | grep /sac/tools/build/linux)" ]; then
	source $whereAmI/../..coolStuff.sh
else
	source $whereAmI/../../sac/tools/coolStuff.sh
fi

######### 0 : Check arguments. #########
if [ ! -z "`echo $whereAmI | grep sac/tools/build`" ]; then
	error_and_usage_and_quit "You can't run the script from sac directory! You must copy the build directory at the root of your game."
fi

if [ $# != 0 ]; then
	usage_and_quit
fi

 info "Compiling.."
 if (!(cmake -DCMAKE_TOOLCHAIN_FILE=../../emscripten.cmake ../..)); then
	 error_and_quit "Error in cmake. Maybe should run with C option?"
 elif (!(make -j4)); then
	 error_and_quit "Error in make"
 fi

