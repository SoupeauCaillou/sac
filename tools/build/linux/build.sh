#!/bin/bash

#how to use the script
export USAGE="$0 [options]"
export OPTIONS="n: simply compile
\tC: remove all cache files (rm -r rm CMakeCache.txt CMakeFiles cmake_install.cmake linux Makefile sac sources 2>/dev/null)
\tr: reset the terminal screen before compiling
\tR: run the app
\td: run&debug the app with gdb (no c option)
\tv: enable log
\tc [all/I/W/E/C] : use colorlog.sh script for colored logs"

export EXAMPLE="'$0 R-v-c-I-W' will compile, launch the app and show only info and warnings in color."
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

if [ $# = 0 ] || [ `echo $1 | grep -- -h` ]; then
	usage_and_quit
fi

# Go to the script dir and retrieve game name
cd $whereAmI
count=`pwd | tr -c -d / | wc -c`
count=`expr $count - 1`
gameName=`pwd | cut -d/ -f$count`

######### 0 : Execute query. #########
	if [ `echo $1 | grep r` ]; then
		reset
	fi

	if [ `echo $1 | grep C` ]; then
		info "Cleaning.."
		rm -r CMakeCache.txt CMakeFiles cmake_install.cmake linux Makefile sac sources 2>/dev/null
	fi

	if [ `echo $1 | grep -e n -e R -e d` ]; then
		info "Compiling.."
		if (!(cmake ../..)); then
			error_and_quit "Error in cmake. Maybe should run with C option?"
		elif (!(make -j4)); then
			error_and_quit "Error in make"
		fi
	fi

	#debug required
	if [ `echo $1 | grep d` ]; then
		#does not work with cgdb yet :(
		info "A bug? Gdb on the way!"
		(echo r; cat) | gdb ./linux/$gameName
	#launch required
	elif [ `echo $1 | grep R` ]; then
		#verbose required
		if [ `echo $1 | grep -e v -e c` ]; then
			#colored logs
			if [ `echo $1 | grep c` ]; then
				info "Launch with colored log."

				if [ $# = 2 ]; then
					./linux/$gameName -v | ./colorlog.sh $2
				else
					info "No arg for color script ?\nUsage: $0 $1 args-for-it" $red
					info "Using 'all' tag there"
					sleep 3
					./linux/$gameName -v | ./colorlog.sh 'all'
				fi
			else
				info "Launch with log."
				./linux/$gameName -v
			fi
		else
			info "Launch game."
			./linux/$gameName
		fi
	fi
