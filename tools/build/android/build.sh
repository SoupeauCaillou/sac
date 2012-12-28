#!/bin/sh

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

check_package() {
	if [ ! -z "`type $1 | grep 'not found'`" ]; then
		info "Please ensure you have added $2 to your PATH variable ($1 program missing)" $red
		exit
	fi
}

#test that the path contains android SDK and android NDK as required
check_package "android" "android-sdk/tools"
check_package "ndk-build" "android-ndk"
check_package "adb" "android-sdk/platform-tools"
check_package "ant" "ant"


if [ ! -z "`echo $whereAmI | grep -e /sac/tools/build`" ]; then
	info "You can't run the script from sac directory! You must copy the build \
directory in the root of the game." $red
	exit 1
fi

if [ $# -gt 2 ] || [ $# = 0 ] || [ `echo $1 | grep -- -h` ]; then
	echo "Usage: $0 menuChoice [optimizeOrNot=yes]"
	echo "	menuchoice:"
	echo "		- n: ndk-build (compile libs and .cpp files)"
	echo "		- c: compile java code (.java)"
	echo "		- i: install on device"
	echo "		- r: run on device"
	echo "	optimizeOrNot:"
	echo "		- yes: ndk-build -j4 and only one ARM version build (armeabi-v7a)"
	echo "		- else: ndk-build and 2 ARMs version"
	echo "\nExample: \"$0 n-c-i yes\" will compile with optimized parameters, then install the app on device."
	exit 1
fi


# *** go to the root dir
if [ ! -z "`echo $whereAmI | grep -e 'sac/tools/build'`" ]; then
	cd $whereAmI/../../../..
else
	cd $whereAmI/../..
fi

# *** get name of the game
count=`pwd | tr -c -d / | wc -c`
count=`expr $count + 1`
nameLower=`pwd | cut -d/ -f $count`
nameUpper=`echo $nameLower | sed 's/^./\u&/'`


if [ ! -z `echo $1 | grep n` ]; then
	info "Building tremor lib..."
	cd sac/libs/tremor; git checkout *; cd ..; ./convert_tremor_asm.sh; cd ../..

	if [ $# != 2 ] || [ ! -z `echo $2 | grep yes` ]; then
		info "ndk-build -j in $PWD"
		ndk-build -j APP_ABI=armeabi-v7a NDK_APPLICATION_MK=android/Application.mk
	else
		info "ndk-build in $PWD"
		ndk-build NDK_APPLICATION_MK=android/Application.mk
	fi
fi
cd sac/libs/tremor; git checkout *; cd ../../..

if [ ! -z `echo $1 | grep c` ]; then
	info "Compiling..."
	android update project -p . -t "android-8" -n $nameUpper --subprojects
	ant debug
fi

if [ ! -z `echo $1 | grep i` ]; then
	info "Installing on device ..."
	ant installd -e
fi

if [ ! -z `echo $1 | grep r` ]; then
	info "Running app $nameUpper..."
	adb shell am start -n net.damsy.soupeaucaillou.$nameLower/net.damsy.soupeaucaillou.$nameLower.${nameUpper}Activity
fi

