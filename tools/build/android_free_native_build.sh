#!/bin/bash

# This script build native part of a game without including 
# non-opensource plugins (Google Play Game Services, etc.)
# It is mainly used for the fdroid build

compilation() {
    ARCHI=$1

    cd build/android-release-$ARCHI
    CMAKE_CONFIG=(-DTARGET=android)
    if [ "$ARCHI" = "x86" ]; then
        CMAKE_CONFIG+=(-DANDROID_ABI=x86)
    fi
    CMAKE_CONFIG+=(-DCMAKE_TOOLCHAIN_FILE=$rootPath/sac/build/cmake/toolchains/android.toolchain.cmake)
    CMAKE_CONFIG+=(-DANDROID_FORCE_ARM_BUILD=ON)
    CMAKE_CONFIG+=(-DCMAKE_BUILD_TYPE=release)
    CMAKE_CONFIG+=(-DUSE_RESTRICTIVE_PLUGINS=OFF)


    cd $rootPath/sac/libs/tremor; git checkout *; ../convert_tremor_asm.sh; cd - 1>/dev/null
    cmake "${CMAKE_CONFIG[@]}" $rootPath
    CPU_INFOS=$(cat /proc/cpuinfo | grep processor | wc -l)
    make -j $CPU_INFOS
    cd $rootPath/sac/libs/tremor; git checkout *; cd - 1>/dev/null

    cd $rootPath
}

# go to the root path
whereAmI=$(cd "$(dirname "$0")" && pwd)
rootPath=$whereAmI/../../..

cd $rootPath

rm -rf build/android-release-arm android-release-x86
mkdir build/android-release-arm android-release-x86

# Generate arm & x86 APK
if ! compilation x86; then
    echo "Error when building x86 version"
    exit 1
elif ! compilation arm; then
    echo "Error when building ARM version"
    exit 2
fi        
