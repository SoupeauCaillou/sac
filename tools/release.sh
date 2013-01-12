#!/bin/sh

#how to use the script
export USAGE="$0 pathOfGame OldName NewName [--preview]"
export OPTIONS="--preview : don't apply changes."
export EXAMPLE="$0 /tmp/heriswap Heriswap Prototype"
#where the script is
whereAmI=`cd "$(dirname "$0")" && pwd`
#import cool stuff
source $whereAmI/coolStuff.sh

######### 0 : Check arguments. #########
	if [ $PWD != $whereAmI ]; then
		error_and_usage_and_quit "Need to launch $0 from its directory"
	fi

######### 1 : Generate languages from google doc #########
	info "Generating languages"
		./xmlLanguageFromTable.sh
	info "Generating languages done."

######### 1 : Build android versions (apks) #########
	info "Building APKs..."

	#go to the root of the game
	cd $whereAmI/../..

	#get name of the game
	count=`pwd | tr -c -d / | wc -c`
	count=`expr $count + 1`
	nameLower=`pwd | cut -d/ -f $count`
	nameUpper=`echo $nameLower | sed 's/^./\u&/'`


	info "Building tremor lib..."
	cd sac/libs/tremor; git checkout *; cd ..; ./convert_tremor_asm.sh; cd ../..


	SCREENres="testk" #to be completed
	for i in $SCREENres; do
		#need to generate the right atlas


		GPUcompression="testj" #to be completed
		for j in $GPUcompression; do
			ARMversions="armeabi-v7a armeabi"
			for k in $ARMversions; do
				info "ndk-build -j in $PWD for $k"
				ndk-build -j APP_ABI=$k NDK_APPLICATION_MK=android/Application.mk

				info "Compiling..."
				android update project -p . -t "android-8" -n $nameUpper --subprojects
				ant debug

				mv bin/$nameUpper-debug.apk bin/$nameUpper-debug-$k-$j-$i.apk
			done
		done
	done

	info "APKs built."

