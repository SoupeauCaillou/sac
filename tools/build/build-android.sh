#!/bin/bash

#how to use the script
export PLATFORM_OPTIONS="\
\t\t- c: compile java code (.java)
\t\t- i: install on device
\t\t- r: run on device
\t\t- l: run adb logcat"

check_necessary() {
    #test that the path contains android SDK and android NDK as required
	check_package "android" "android-sdk/tools"
	check_package "ndk-build" "android-ndk"
	check_package "adb" "android-sdk/platform-tools"
	check_package "ant" "ant"
}

compilation_before() {
	info "Building tremor lib..."
	cd sac/libs/tremor; git checkout *; sh ../convert_tremor_asm.sh; cd -

    info "Adding specific toolchain for android..."
    CMAKE_CONFIG=$CMAKE_CONFIG" -DANDROID_TOOLCHAIN_NAME=arm-linux-androideabi-4.7\
    -DCMAKE_TOOLCHAIN_FILE=$rootPath/sac/build/cmake/toolchains/android.toolchain.cmake\
    -DANDROID_NATIVE_API_LEVEL=9"
}

compilation_after() {
    info "Cleaning tremor directory..."
    cd $rootPath/sac/libs/tremor; git checkout *; cd -

    #android update project -p . -t "android-8" -n $nameUpper --subprojects
    #ant debug
}

launch_the_application() {
    if [ ! -z $(echo $1 | grep i) ]; then
        info "Installing on device ..."
        ant installd -e
    fi

    if [ ! -z $(echo $1 | grep r) ]; then
        info "Running app $gameName..."

        gameNameLower=$(echo $gameName | sed 's/\u&/^./')

        adb shell am start -n net.damsy.soupeaucaillou.$gameNameLower/net.damsy.soupeaucaillou.$gameNameLower.${gameName}Activity
    fi

    if [ ! -z $(echo $1 | grep l) ]; then
        info "Launching adb logcat..."
        adb logcat -C | grep sac
    fi
}















