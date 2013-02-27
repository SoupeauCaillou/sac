#!/bin/bash

#how to use the script
export USAGE="$0 menuChoice [optimizeOrNot=yes]"
export OPTIONS="menuchoice:
\t\t- n: ndk-build (compile libs and .cpp files)
\t\t- c: compile java code (.java)
\t\t- i: install on device
\t\t- r: run on device
\t\t- l: run adb logcat
\toptimizeOrNot:
\t\t- yes: ndk-build -j4 and only one ARM version build (armeabi-v7a)
\t\t- else: ndk-build and 2 ARMs version"
export EXAMPLE="'$0 n-c-i yes' will compile with optimized parameters, then install the app on device."

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

######### 0 : Check requirements. #########
#test that the path contains android SDK and android NDK as required
	check_package "android" "android-sdk/tools"
	check_package "ndk-build" "android-ndk"
	check_package "adb" "android-sdk/platform-tools"
	check_package "ant" "ant"

	if [ -z "$(pwd | grep /sac/tools/build)" ]; then
		error_and_quit "The script must be in sac/tools/build !"
	fi

######### 1 : Check arguments. #########
	if [ $# != 1 ] || [ ! -z $(echo $1 | grep -- -h) ]; then
		usage_and_quit
	fi

######### 2 : Create build dir. #########
	rootPath=$whereAmI"/../../.."

	mkdir -p $rootPath/build/android

######### 3 : Go into build/emscripten #########
	cd $rootPath

######### 4 : Execute query. #########
	count=$(pwd | tr -c -d / | wc -c)
	count=`expr $count + 1`
	nameLower=$(pwd | cut -d/ -f $count)
	nameUpper=$(echo $nameLower | sed 's/^./\u&/')

	echo $nameLower $nameUpper
	if [ ! -z `echo $1 | grep n` ]; then
		info "Building tremor lib..."
		cd $rootPath/sac/libs/tremor; git checkout *; cd ..; ./convert_tremor_asm.sh; cd ../..

		if [ $# != 2 ] || [ ! -z `echo $2 | grep yes` ]; then
			opti="-j APP_ABI=armeabi-v7a"
		else
			opti="-j4"
		fi

		info "ndk-build $opti in $PWD"
		if (!(ndk-build $opti NDK_APPLICATION_MK=android/Application.mk)); then
				info "ndk-build failed" $red
				exit 1
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

	if [ ! -z `echo $1 | grep l` ]; then
		info "Launching adb logcat..."
		adb logcat -C | grep sac
	fi














