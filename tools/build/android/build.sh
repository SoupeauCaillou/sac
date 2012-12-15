#!/bin/sh

#test the command line
if [ "$0" != "./build.sh" ]; then
	echo "Please don't execute $0 but: ./build.sh"
	echo "Abort."
	exit
fi

if [ ! -f "build.sh" ] || [ $# != 2 ] || [ `echo $1 | grep -- -h` ]; then
	echo "Usage: $0 menuChoice optimizeOrNot"
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
esc=""

  
PATH_sdk=$HOME/code/c/tools/android-sdk-linux
PATH_ndk=$HOME/code/c/tools/android-ndk
echo "PATH_sdk=$PATH_sdk"
echo "PATH_ndk=$PATH_ndk"
echo "${esc}[31mIf it's not valid, please edit them in the script and try again${esc}[0m"
	
# *** go to the root dir
cd ../..

# *** get name of the game
count=`pwd | tr -c -d / | wc -c`
count=`expr $count + 1`
nameLower=`pwd | cut -d/ -f $count`
nameUpper=`echo $nameLower | sed 's/^./\u&/'`


if [ ! -z `echo $1 | grep n` ]; then
	echo "Building tremor lib..."
	cd sac/libs/tremor; git checkout *; cd ..; ./convert_tremor_asm.sh; cd ../..
	
	if [ ! -z `echo $2 | grep yes` ]; then
		echo "ndk-build -j in $PWD"
		$PATH_ndk/ndk-build -j APP_ABI=armeabi-v7a NDK_APPLICATION_MK=android/Application.mk
	else
		echo "ndk-build in $PWD"
		$PATH_ndk/ndk-build NDK_APPLICATION_MK=android/Application.mk
	fi
fi

if [ ! -z `echo $1 | grep c` ]; then
	echo "Compiling..."
	$PATH_sdk/tools/android update project -p . -t "android-8" -n $nameUpper --subprojects
	ant debug
fi

if [ ! -z `echo $1 | grep i` ]; then
	echo "Installing on device ..."
	ant installd -e
fi

if [ ! -z `echo $1 | grep r` ]; then
	echo "Running app $nameUpper..."
	adb shell am start -n net.damsy.soupeaucaillou.$nameLower/net.damsy.soupeaucaillou.$nameLower.${nameUpper}Activity
fi


# *** cd build/android
