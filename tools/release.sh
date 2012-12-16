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

# step 0 : check we are at the right place
if [ $PWD != $whereAmI ]; then
	info "Need to launch $0 from its directory" "$red"
	exit
fi


# step : generate languages from google doc
info "Generating languages"
	#- ./xmlLanguageFromTable.sh
info "Generating languages done."

# step : build android versions (apks)
info "Building APKs..."

	cd $whereAmI/../..

	count=`pwd | tr -c -d / | wc -c`
	count=`expr $count + 1`
	nameLower=`pwd | cut -d/ -f $count`
	nameUpper=`echo $nameLower | sed 's/^./\u&/'`


	info "Building tremor lib..."
	cd sac/libs/tremor; git checkout *; cd ..; ./convert_tremor_asm.sh; cd ../..


	ARMversions="armeabi-v7a armeabi"
	for i in $ARMversions; do
		GPUcompression="testj" #to be completed
		for j in $GPUcompression; do
			SCREENres="testk" #to be completed
			for k in $SCREENres; do
				info "ndk-build -j in $PWD for $i"
				ndk-build -j APP_ABI=$i NDK_APPLICATION_MK=android/Application.mk

				info "Compiling..."
				android update project -p . -t "android-8" -n $nameUpper --subprojects
				ant debug

				mv bin/$nameUpper-debug.apk bin/$nameUpper-debug-$i-$j-$k.apk
			done
		done
	done

info "APKs built."

