#! /bin/sh

# *** check we are in build/linux
if [ -z `pwd | grep "/build/linux"` ]; then
	echo "Wrong dir. Please go to $game/build/linux path to use the script."
	echo "You are currently in $PWD"
	exit 1
fi
	

if [ $# != 1 ] || [`echo $1 | grep -- -h` ]; then
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
	echo "cleaning"
	rm -r CMakeCache.txt CMakeFiles cmake_install.cmake linux Makefile sac sources 2>/dev/null
fi


count=`pwd | tr -c -d / | wc -c`
count=`expr $count - 1`
gameName=`pwd | cut -d/ -f$count`

#delete the executable (in case of compilation errors)
rm -f linux/$gameName

echo "compiling"
cmake ../..
make -j





if [ `echo $1 | grep d` ]; then
	(echo r; cat) | gdb ./linux/$gameName
elif [ `echo $1 | grep R` ]; then
	if [ `echo $1 | grep -e v -e c` ]; then
		if [ `echo $1 | grep c` ]; then
			./linux/$gameName -v | ./colorlog.sh $1
		else
			./linux/$gameName -v
		fi
	else
		./linux/$gameName
	fi
fi
