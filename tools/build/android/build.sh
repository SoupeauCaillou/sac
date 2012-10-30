#!/bin/bash

#test the command line
if [ "$0" != "./build.sh" ]; then
	echo "Please don't execute $1 but: \t./build.sh"
	echo "Abort."
	exit
fi
count=`pwd | tr -c -d / | wc -c`
count=`expr $count - 1`

nameLower=`pwd | cut -d/ -f $count`
nameUpper=`echo $nameLower | sed 's/^./\u&/'`

PATH_sdk=$HOME/code/c/tools/android-sdk-linux
PATH_ndk=$HOME/code/c/tools/android-ndk

echo "PATH_sdk=$PATH_sdk"
echo "PATH_ndk=$PATH_ndk"
echo "Valid path ? (y/N)"
read ichoix
if [[ "$ichoix" != "y" && "$ichoix" != "Y" ]]; then
	echo "Edit them then come back"
	exit
fi

echo "What to do ? \n
	1. ndk-build
	2. (1) + compile
	3. (2) + install on device
	4. (3) + run
	5. run
	Else. quit"
read ichoix

#verify choice is good
[ $ichoix -eq 0 ] 2> /dev/null 
# isn't a number
if ! [ $? -eq 0 -o $? -eq 1 ]; then
	echo "unrecognized choice, abort"
	exit
#is in range ?
else
	if [ "$ichoix" -lt 1 ] || [ "$ichoix" -gt 5 ]; then
		echo "wrong number"
		exit
	fi
fi

cd ../..
if [ "$ichoix" -gt 0 ] && [ "$ichoix" -lt 5 ]; then
	echo "Optimize ? (ndk-build -j and only one ARM version build ?) (Y/n) ?"
	read optimize
	
	if [ "$optimize" = "n" ]; then
		echo "ndk-build in $PWD"
		$PATH_ndk/ndk-build NDK_APPLICATION_MK=android/Application.mk
	else
		echo "ndk-build -j in $PWD"
		$PATH_ndk/ndk-build -j APP_ABI=armeabi-v7a NDK_APPLICATION_MK=android/Application.mk
	fi
fi
if [ "$ichoix" -gt 1 ] && [ "$ichoix" -lt 5 ]; then
	echo "compiling"
	$PATH_sdk/tools/android update project -p . -t 4 -n $nameUpper  --subprojects
	ant debug
fi
if [ "$ichoix" -gt 2 ] && [ "$ichoix" -lt 5 ]; then
	echo "installing on device …"
	ant installd -e
fi
if [ "$ichoix" = 4 ] || [ "$ichoix" = 5 ]; then
	echo "launching app"
	adb shell am start -n net.damsy.soupeaucaillou.$nameLower/net.damsy.soupeaucaillou.$nameLower.${nameUpper}Activity
fi
cd build/android
