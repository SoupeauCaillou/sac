#!/bin/bash

#how to use the script
export PLATFORM_OPTIONS="\
\td: run&debug the app with gdb
\t-arm: build ARM version
\t-x86: build x86 version
\t-a|-atlas: generate all atlas from unprepared_assets directory
\t-g|-gradle: use gradle(android studio) instead of ant(eclipse) - NOT HANDLED YET
\t-p|-project: regenerate ant files and run it
\t-i|-install: install on device
\t-u|-uninstall: uninstall game from device
\t-l|-log: run adb logcat
\t-s|-stacktrace: show latest dump crash trace
\t-c|-clean: remove all bin/ and gen/ directories (in sac too!)
\t--v|--validation \"path-to-keystore-from-here\": sign APK with keystore passed as argument (abs or relative path)"


parse_arguments() {
    ARCHI=arm
    GENERATE_ATLAS=0
    USE_GRADLE=0
    REGENERATE_ANT_FILES=0
    INSTALL_ON_DEVICE=0
    UNINSTALL_FROM_DEVICE=0
    RUN_LOGCAT=0
    STACK_TRACE=0
    SIGN_APK=
    FORCE_CLEAN=0
    while [ "$1" != "" ]; do
        case $1 in
            #ignore higher level commands
            "-h" | "-help" | "-release" | "-debug")
                # info "Ignoring '$1' (low lvl)"
                ;;
            #ignore higher level commands
            "--c" | "--cmakeconfig" | "--t" | "--target")
                # info "Ignoring '$1' and its arg '$2' (low lvl)"
                shift
                ;;
            "-a" | "-atlas")
                GENERATE_ATLAS=1
                TARGETS=$TARGETS" "
                ;;
            "-g" | "-gradle")
                USE_GRADLE=1
                TARGETS=$TARGETS" "
                ;;
            "-p" | "-project")
                REGENERATE_ANT_FILES=1
                TARGETS=$TARGETS"n"
                ;;
            "-i" | "-install")
                INSTALL_ON_DEVICE=1
                TARGETS=$TARGETS" "
                ;;
            "-u" | "-uninstall")
                UNINSTALL_FROM_DEVICE=1
                TARGETS=$TARGETS" "
                ;;
            "-l" | "-log")
                RUN_LOGCAT=1
                TARGETS=$TARGETS" "
                ;;
            "-s" | "-stacktrace")
                TARGETS=$TARGETS" "
                STACK_TRACE=1
                ;;
            "-c" | "-clean")
                TARGETS=$TARGETS" "
                FORCE_CLEAN=1
                ;;
            "-arm")
                ARCHI=arm
                ;;
            "-x86")
                ARCHI=x86
                ;;
            "--v" | "--validation")
                TARGETS=$TARGETS" "
                shift
                SIGN_APK=$1
                ;;
            --*)
                info "Unknown option '$1', ignoring it and its arg '$2'" $red
                shift
                ;;
            -*)
                info "Unknown option '$1', ignoring it" $red
                ;;
        esac
        shift
    done
}

check_necessary() {
    check_environment_variable 'ANDROID_HOME'
    check_environment_variable 'ANDROID_NDK'

    # Add to your PATH environement variable the location of:
        # - ndk-build (ANDROID_NDK)
        # - android (ANDROID_HOME/tools)
        # - adb (ANDROID_HOME/platform-tools)
    if ! is_package_installed android; then
        export PATH=$PATH:$ANDROID_HOME/tools
    fi
    if ! is_package_installed adb; then
        export PATH=$PATH:$ANDROID_HOME/platform-tools
    fi
    if ! is_package_installed ndk-build; then
        export PATH=$PATH:$ANDROID_NDK
    fi

    #test that the path contains android SDK and android NDK as required
	check_package_in_PATH "android" "android-sdk/tools"
    check_package_in_PATH "ndk-build" "android-ndk"
	check_package_in_PATH "adb" "android-sdk/platform-tools"
	check_package "ant"

    #we need 32 bits library to use the NDK tools. If the library is not
    #installed we will get a "No such file or directory" when executing any tool
    aapt_location=$(find $ANDROID_HOME -name 'aapt' | head -n 1)
    $aapt_location &>/dev/null
    # "No such file or directory" code is 127
    if [ $? = 127 ]; then
        error_and_quit "The android NDK requires 32 bits compatibility library. Please ensure you have installed ia32-libs"
    fi

    #change build dir!
    builddir=$builddir-$ARCHI
}

init() {
    if [ $FORCE_CLEAN = 1 ]; then
        to_trash_destination=$(find $rootPath -type d  -name bin -o -name gen)
        if [ ! -z "$to_trash_destination" ]; then
            info "Removing all bin/ and gen/ directories ($(echo $to_trash_destination | tr ' ' '\n' | rev | cut -d/ -f1-2 | rev | tr '\n' ' '))"
            rm -r $to_trash_destination
        else
            info "Asked to remove all bin/ and gen/ directories but I could not find any"
        fi
    fi

    if [ $GENERATE_ATLAS = 1 ]; then
        info "Generating atlas..."
        $rootPath/sac/tools/generate_atlas.sh $rootPath/unprepared_assets/*
    fi
}

compilation_before() {
    info "Building tremor lib..."
    cd $rootPath/sac/libs/tremor; git checkout *; ../convert_tremor_asm.sh; cd - 1>/dev/null
    info "Adding specific toolchain for android..."

    if [ "$ARCHI" = "x86" ]; then
        CMAKE_CONFIG+=(-DANDROID_ABI=x86)
    fi

    CMAKE_CONFIG+=(-DCMAKE_TOOLCHAIN_FILE=$rootPath/sac/build/cmake/toolchains/android.toolchain.cmake)
    CMAKE_CONFIG+=(-DANDROID_FORCE_ARM_BUILD=ON)
    CMAKE_CONFIG+=(-DUSE_GRADLE=$USE_GRADLE)
}

compilation_after() {
    info "Cleaning tremor directory..."
    cd $rootPath/sac/libs/tremor; git checkout *; cd - 1>/dev/null

    if [ $USE_GRADLE = 1 ]; then
        if [ ! -f $rootPath/libs/armeabi-v7a.jar ]; then
            error_and_quit "Missing libs/armeabi.jar (did you forgot to compile?)!"
        fi
        info "TODO for gradle" $red
    else
        rm $rootPath/libs/armeabi-v7a.jar 2>/dev/null
    fi

    if [ $REGENERATE_ANT_FILES = 1 ]; then
        info "Updating android project"
        cd $rootPath

        # -t "android-8" is required for installLocation,
        # glEsVersion, targetSdkVersion and allowBackup attributes
        # but currently all ndk <= 9 produce buggy builds for android-8, so use android-10
        if ! android update project -p . -t "android-10" -n $gameName --subprojects; then
            error_and_quit "Error while updating project"
        fi

        # if ant failed, redo it in verbose mode
        if ! ant -q release; then
            ant release
            error_and_quit "Ant failed - see above for the reason"
        fi
    fi
}

get_APK_name() {
    if [ ! -z "$APK" ] && [ -e "$APK" ]; then
        info "APK variable already set to '$APK'. Aborting searching a new one."
    else
        if [ -f "$rootPath/bin/$gameName-release.apk" ]; then
            APK=$rootPath/bin/$gameName-release.apk
            info "Found a release APK ($(echo $APK | sed 's|.*/||'))..."
        elif [ -f "$rootPath/bin/$gameName.apk" ]; then
            APK=$rootPath/bin/$gameName.apk
            info "Found a release APK ($(echo $APK | sed 's|.*/||'))..."
        elif [ -f "$rootPath/bin/${gameName}-debug.apk" ]; then
            APK=$rootPath/bin/${gameName}-debug.apk
            info "Found a debug APK ($(echo $APK | sed 's|.*/||'))..."
        elif [ -d $rootPath/bin ]; then
            APK=$(find $rootPath/bin -name "$gameName*.apk")
            if [ -z "$APK" ]; then
                 error_and_quit "Could not find any APK"
            fi
            APK=$(echo $APK | head -n1)
            info "Couldn't find a usual name for the apk... will use '$(echo $APK | sed 's|.*/||')'" $orange
        else
            error_and_quit "Could not find any APK (did you forget to use -p option?)"
        fi
    fi
}

launch_the_application() {
    if [ $UNINSTALL_FROM_DEVICE = 1 ]; then
        info "Uninstalling from device..."
        adb uninstall $(cat $rootPath/AndroidManifest.xml | grep package | cut -d "=" -f 2 | tr -d '"')
    fi


    if [ ! -z "$SIGN_APK" ]; then
        cd $fromWhereAmIBeingCalled
        if [ ! -e "$SIGN_APK" ]; then
            error_and_quit "File not found: '$SIGN_APK'. Beware: if it's a relative path, it must be relative to $fromWhereAmIBeingCalled location!"
        fi
        info "Did you know that you can made it automatically? Simply add the following lines in $rootPath/project.properties:
    key.store=$SIGN_APK
    key.alias=your_key_alias_name
    key.store.password=your_keystore_passwd
    key.alias.password=your_key_passwd
Continuing..."

        get_APK_name
        info "Signing APK '$APK' with keystore located at $SIGN_APK"

        info "What is the key alias? (Can be found with 'keytool -keystore $SIGN_APK -list')" $blue
        read key_alias_name
        if (!(jarsigner -verbose -keystore $SIGN_APK $APK $key_alias_name)); then
            error_and_quit "Error when signing APK."
        fi
        rm ${gameName}-release-signed-with-sac.apk -f
        if (!(zipalign -v 4 $APK ${gameName}-release-signed-with-sac.apk)); then
            error_and_quit "Error when zipaligning APK."
        fi
        APK=${gameName}-release-signed-with-sac.apk
        info "Successfully created signed APK '$APK'"
    fi


    cd $rootPath
    if [ $INSTALL_ON_DEVICE = 1 ]; then
        get_APK_name
        info "Installing '$APK' on device..."
        if [ $USE_GRADLE = 1 ]; then
            info "TODO for gradle" $red
        else
            if (!(adb install -r $APK)); then
                error_and_quit "Could not install $APK on device!"
            fi
        fi
    fi

    packageName=$(grep 'package=' $rootPath/AndroidManifest.xml | sed 's/package=/~/' | cut -d'~' -f2 | cut -d ' ' -f 1 | tr -d '"')
    activityName=$(grep '<activity' $rootPath/AndroidManifest.xml | sed 's/android:name/~/' | cut -d'~' -f2 | cut -d ' ' -f 1 | tr -d '=>"')

    #debug required
    if [ ! -z "$(echo $TARGETS | grep d)" ]; then
        info "A bug? gdb on the way!"
        if ! ndk-gdb; then
            info "Note: this does not work with Android<2.3 and Cyanogen devices!" $orange
            error_and_quit "Could not start gdb. Did your build a debug version?"
        fi
    #launch required
    elif [ ! -z $(echo $1 | grep r) ]; then
        info "Running app '$gameName'..."

        if (!(adb shell am start -n $packageName/$activityName)); then
            error_and_quit "Could not run $gameName!"
        fi
    fi

    if [ $RUN_LOGCAT = 1 ]; then
        info "Launching adb logcat..."
        adb logcat -C | grep sac
    fi

    if [ $STACK_TRACE = 1 ]; then
        info "Printing latest dump crashes"
        check_package_in_PATH "ndk-stack" "android-ndk"
        adb logcat | ndk-stack -sym $rootPath/libs/armeabi-v7a
    fi
}
