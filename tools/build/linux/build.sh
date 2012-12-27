#! /bin/sh

###### Cool things ##############################
#colors
reset="[0m"
red="[1m[31m"
green="[1m[32m"

info() {
	if [ $# = 1 ]; then
		echo "${green}$1${reset}"
	else
		echo "$2$1${reset}"
	fi
}

#get location of the script
whereAmI=`cd "$(dirname "$0")" && pwd`


##########End of cool things ######################


if [ ! -z "`echo $whereAmI | grep sac/tools`" ]; then
	echo "You can't run the script from sac directory! You must copy the build \
directory in the root of the game."
	exit 1
else
	# *** go to the script dir
	cd $whereAmI
fi


if [ $# == 0 || [`echo $1 | grep -- -h` ]; then
	echo "Usage: $0 options"
	echo "	- n: simply compile"
	echo "	- C: remove all cache files (rm -r rm CMakeCache.txt CMakeFiles cmake_install.cmake linux Makefile sac sources 2>/dev/null)"
	echo "	- r: reset the terminal screen before compiling"
	echo "	- R: run the app"
	echo "	- d: run&debug the app with gdb (no c option)"
	echo "	- v: enable log"
	echo "	- c [all/I/W/E/C] : use colorlog.sh script for colored logs"
	echo "\nExample: \"$0 R-v-c-I-W\" will compile, launch the app and show only info and warnings in color."
	exit 1
fi

if [ `echo $1 | grep r` ]; then
	reset
fi

if [ `echo $1 | grep C` ]; then
	info "Cleaning.."
	rm -r CMakeCache.txt CMakeFiles cmake_install.cmake linux Makefile sac sources 2>/dev/null
fi


count=`pwd | tr -c -d / | wc -c`
count=`expr $count - 1`
gameName=`pwd | cut -d/ -f$count`

#delete the executable (in case of compilation errors)
rm -f linux/$gameName

info "Compiling.."

if (!(cmake ../..)); then
	info "Error in cmake. Maybe should run with C option?" $red
	exit
elif (!(make -j4)); then
	info "Error in make" $red
	exit
fi

if [ `echo $1 | grep d` ]; then
	#permet de lancer gdb avec directement "r" en paramètre
	#ne fonctionne pas pour cgdb :(
	info "A bug? Gdb on the way!"
	(echo r; cat) | gdb ./linux/$gameName
elif [ `echo $1 | grep R` ]; then
	if [ `echo $1 | grep -e v -e c` ]; then
		if [ `echo $1 | grep c` ]; then
			info "Launch with colored log."

			if [ $# = 2 ]; then
				./linux/$gameName -v | ./colorlog.sh $2
			else
				info "No arg for color script ?\nUsage: $0 $1 args-for-it" $red
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
