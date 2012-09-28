#!/bin/bash

if [ $0 != "./android_build.sh" ]; then
	echo "Please run the script ./android_build.sh from sac/tools/. Abort."
	exit
fi
cd ../..


PATH_sdk=$HOME/code/c/tools/android-sdk-linux
PATH_ndk=$HOME/code/c/tools/android-ndk

echo "SDK PATH : $PATH_sdk"
echo "NDK PATH : $PATH_ndk"
echo "Valid ? (y/N)"
read confir
if [[ "$confir" != "y" && "$confir" != "Y" ]]; then
	echo "Edit the path and rerun the script. Abort."
	exit
fi


echo "What to do ?
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
	$PATH_sdk/tools/android update project -p . -t 4 -n Heriswap  --subprojects
	ant debug
fi
if [ "$ichoix" -gt 2 ] && [ "$ichoix" -lt 5 ]; then
	echo "installing on device …"
	ant installd -e
fi
if [ "$ichoix" = 4 ] || [ "$ichoix" = 5 ]; then
	echo "launching app"
	adb shell am start -n net.damsy.soupeaucaillou.heriswap/net.damsy.soupeaucaillou.heriswap.HeriswapActivity
fi
cd build/android
