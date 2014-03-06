#!/bin/bash

#from where are we calling it
fromWhereAmIBeingCalled=$PWD
#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
cd $whereAmI
#if we executed a linked script; go to the real one
if [  -h $0 ]; then
	whereAmI+=/$(dirname $(readlink $0))
	cd $whereAmI
fi
rootPath=$whereAmI"/../../.."
gameName=$(cat $rootPath/CMakeLists.txt | grep 'project(' | cut -d '(' -f2 | tr -d ')')

#import cool stuff
source ../cool_stuff.sh

export SAC_USAGE="$0 [options]. This will generate apk which can be uploaded on google console https://play.google.com/apps/publish/"
export SAC_OPTIONS="\
--vn|--versionName value: change AndroidManifest version name to 'value'
-i|-install: uninstall from device and install the generated apk
-r|-run: run the APK from device
--args: arguments to give to build-all script (see it for available options)
-t|-tag: add a git tag for the current version
"

cd $rootPath

######### 0 : Read arguments. #########
    OTHERS_ARGS=()
    versionName=""
	DO_GIT_TAG=0
	RUN=""
    while [ "$1" != "" ]; do
        case $1 in
            "-h" | "-help")
                usage_and_quit
                ;;
            "-t" | "-tag")
				DO_GIT_TAG=1
				;;
			"-r" | "-run")
				RUN+=" r"
				;;
			"-i" | "-install")
				RUN+=" -u -i"
				;;
            "--args")
                shift
                OTHERS_ARGS+=($1)
                ;;
             "--vn" | "--versionName")
                shift
                versionName=$1
                ;;
            --*)
                ARGS+=" "$1
                shift
                ARGS+=" "$1
                ;;
            -*)
                ARGS+=" "$1
                ;;
            *)
                TARGETS=$TARGETS$1
        esac
        shift
    done


######### 1 : Check requirements. #########
if [ $DO_GIT_TAG = 1 ]; then
	if [ -z "$versionName" ]; then
		#dont allow to create a git tag if user didn't change the version name (because
		#there must be already an existing apk with this tag)
		error_and_quit "You can't create a git tag if you don't change the version name"
	elif [ ! -z "$(git status -s)" ]; then
		error_and_quit "Please get a clean git copy before running the script: $(git status -s)"
	fi
fi

######### 2 : Replace version code in Android Manifest to current date. #########
info "Replacing version code in Manifest..."
versionCode=$(date +"%y%m%d%H%M")
sed -i "s/android:versionCode=.*/android:versionCode=\"$versionCode\"/" AndroidManifest.xml
#sed -i "s/package=.*/package=\"net.damsy.soupeaucaillou.sactestonly\"/" AndroidManifest.xml
# sed -i "s/android:label=\"[^\"]*\"/android:label=\"0sac test only\"/" AndroidManifest.xml

######### 3 : Replace version name in Android Manifest if user wants. #########
if [ ! -z "$versionName" ]; then
	info "Updating version name to $versionName" 
	sed -i "s/android:versionName=.*/android:versionName=\"$versionName\"/" AndroidManifest.xml		
else
    versionName=$(grep 'android:versionName' AndroidManifest.xml | cut -d= -f2 | tr -d '"')
	info "Keeping version name to $versionName"
fi

######### 4 : compile game for both versions. #########
if ! ./sac/tools/build/build-all.sh --target android --g ninja -x86 -release n -c $OTHERS_ARGS; then
    error_and_quit "Error when building x86 version"
elif ! ./sac/tools/build/build-all.sh --target android --g ninja -arm -release -p n -c $OTHERS_ARGS; then
    error_and_quit "Error when building ARM version"
fi        
info "Saving apk to $rootPath/bin/$gameName-$versionCode-$versionName.apk"
cp $rootPath/bin/$gameName-release.apk $rootPath/bin/$gameName-$versionCode-$versionName.apk

######### 5 : generate tag. #########
if [ $DO_GIT_TAG = 1 ]; then
	git tag -a $versionName -m "version $versionName"
	info "Do not forget to push it with 'git push origin $versionName'!"
fi

######### 6 : run apk on device if needed. #########
if [ ! -z "$RUN" ]; then
	./sac/tools/build/build-all.sh --target android $RUN
fi
