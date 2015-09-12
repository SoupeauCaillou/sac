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
    ret=0
    archi=arm
    generate_atlas=0
    use_gradle=0
    regenerate_ant_files=0
    install_on_device=0
    uninstall_from_device=0
    run_logcat=0
    stack_trace=0
    sign_apk=
    force_clean=0
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
                generate_atlas=1
                targets=$targets" "
                ;;
            "-g" | "-gradle")
                use_gradle=1
                error_and_quit "Sorry, gradle build not supported anymore for yet"
                targets=$targets" "
                ;;
            "-p" | "-project")
                regenerate_ant_files=1
                targets=$targets"n"
                ;;
            "-i" | "-install")
                install_on_device=1
                targets=$targets" "
                ;;
            "-u" | "-uninstall")
                uninstall_from_device=1
                targets=$targets" "
                ;;
            "-l" | "-log")
                run_logcat=1
                targets=$targets" "
                ;;
            "-s" | "-stacktrace")
                targets=$targets" "
                stack_trace=1
                ;;
            "-c" | "-clean")
                targets=$targets" "
                force_clean=1
                ;;
            "-arm")
                archi=arm
                ;;
            "-x86")
                archi=x86
                ;;
            "--v" | "--validation")
                targets=$targets" "
                shift
                sign_apk=$1
                ;;
            --*)
                ret=$(($ret + 1))
                info "Unknown option '$1', ignoring it and its arg '$2'" $red
                shift
                ;;
            -*)
                ret=$(($ret + 1))
                info "Unknown option '$1', ignoring it" $red
                ;;
        esac
        shift
    done
    return $ret
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
    builddir=$builddir-$archi
}

init() {
    if [ $force_clean = 1 ]; then
        to_trash_destination=$(find $rootPath -type d  -name bin -o -name gen)
        if [ ! -z "$to_trash_destination" ]; then
            info "Removing all bin/ and gen/ directories ($(echo $to_trash_destination | tr ' ' '\n' | rev | cut -d/ -f1-2 | rev | tr '\n' ' '))"
            rm -r $to_trash_destination
        else
            info "Asked to remove all bin/ and gen/ directories but I could not find any"
        fi
    fi

    if [ $generate_atlas = 1 ]; then
        info "Generating atlas..."
        $rootPath/sac/tools/generate_atlas.sh $rootPath/unprepared_assets/*
    fi
}

compilation_before() {
    info "Adding specific toolchain for android..."

    if [ "$archi" = "x86" ]; then
        cmake_config+=(-DANDROID_ABI=x86)
    fi

    cmake_config+=(-DCMAKE_TOOLCHAIN_FILE=$rootPath/sac/cmake/toolchains/android.toolchain.cmake)
    cmake_config+=(-DANDROID_FORCE_ARM_BUILD=ON)
    # cmake_config+=(-DUSE_GRADLE=$use_gradle)
}

compilation_after() {
    :
}

android_compilation() {
    if [ $use_gradle = 1 ]; then
        if [ ! -f $rootPath/android/libs/armeabi-v7a.jar ]; then
            error_and_quit "Missing libs/armeabi.jar (did you forgot to compile?)!"
        fi
        info "TODO for gradle" $red
    else
        rm $rootPath/android/libs/armeabi-v7a.jar 2>/dev/null
    fi

    if [ $regenerate_ant_files = 1 ]; then
        info "Updating android project"
        cd $rootPath/android

        # -t "android-8" is required for installLocation, glEsVersion, targetSdkVersion and allowBackup attributes
        # but currently all ndk <= 9 produce buggy builds for android-8, so use android-10
        # -n $gameName was removed because build.xml is overwritten with it, even if <!-- version-tag:custom --> is
        # provided... but we need it to automatically sign APK with release keystore which is stored outside of repository!!!
        if ! android update project -p . -t "android-10" --subprojects; then
            error_and_quit "Error while updating project. You might try the following:\
             android update project -p . -t \"android-10\" --subprojects -n $gameName"
        fi

        info "Running ant $cmake_build_type"
        if ! ant $cmake_build_type; then
            error_and_quit "Ant failed - see above for the reason"
        fi
    fi
}

get_APK_name() {
    if [ ! -z "$apk" ] && [ -e "$apk" ]; then
        info "APK variable already set to '$apk'. Aborting searching a new one."
    else
        if [ -f "$rootPath/android/bin/$gameName-release.apk" ]; then
            apk=$rootPath/android/bin/$gameName-release.apk
            info "Found a release APK ($(echo $apk | sed 's|.*/||'))..."
        elif [ -f "$rootPath/android/bin/$gameName.apk" ]; then
            apk=$rootPath/android/bin/$gameName.apk
            info "Found a release APK ($(echo $apk | sed 's|.*/||'))..."
        elif [ -f "$rootPath/android/bin/${gameName}-debug.apk" ]; then
            apk=$rootPath/android/bin/${gameName}-debug.apk
            info "Found a debug APK ($(echo $apk | sed 's|.*/||'))..."
        elif [ -d $rootPath/android/bin ]; then
            apk=$(find $rootPath/android/bin -name "$gameName*.apk")
            if [ -z "$apk" ]; then
                 error_and_quit "Could not find any APK"
            fi
            apk=$(echo $apk | head -n1)
            info "Couldn't find a usual name for the apk... will use '$(echo $apk | sed 's|.*/||')'" $orange
        else
            error_and_quit "Could not find any APK (did you forget to use -p option?)"
        fi
    fi
}

launch_the_application() {
    android_compilation

    if [ $uninstall_from_device = 1 ]; then
        info "Uninstalling from device..."
        adb uninstall $(cat $rootPath/android/AndroidManifest.xml | grep package | cut -d "=" -f 2 | tr -d '"')
    fi


    if [ ! -z "$sign_apk" ]; then
        cd $fromWhereAmIBeingCalled
        if [ ! -e "$sign_apk" ]; then
            error_and_quit "File not found: '$sign_apk'. Beware: if it's a relative path, it must be relative to $fromWhereAmIBeingCalled location!"
        fi
        info "Did you know that you can made it automatically? Simply add the following lines in $rootPath/android/project.properties:
    key.store=$sign_apk
    key.alias=your_key_alias_name
    key.store.password=your_keystore_passwd
    key.alias.password=your_key_passwd
Continuing..."

        get_APK_name
        info "Signing APK '$apk' with keystore located at $sign_apk"

        info "What is the key alias? (Can be found with 'keytool -keystore $sign_apk -list')" $blue
        read key_alias_name
        if (!(jarsigner -verbose -keystore $sign_apk $apk $key_alias_name)); then
            error_and_quit "Error when signing APK."
        fi
        rm ${gameName}-release-signed-with-sac.apk -f
        if (!(zipalign -v 4 $apk ${gameName}-release-signed-with-sac.apk)); then
            error_and_quit "Error when zipaligning APK."
        fi
        apk=${gameName}-release-signed-with-sac.apk
        info "Successfully created signed APK '$apk'"
    fi


    cd $rootPath/android/
    if [ $install_on_device = 1 ]; then
        get_APK_name
        info "Installing '$apk' on device..."
        if [ $use_gradle = 1 ]; then
            info "TODO for gradle" $red
        else
            if (!(adb install -r $apk)); then
                error_and_quit "Could not install $apk on device!"
            fi
        fi
    fi

    packageName=$(grep -o "package=.*\b" $rootPath/android/bin/AndroidManifest.xml | cut -d '"' -f 2)
    activityName=$(grep -o "<activity.*android:name=\"[^\"]*" $rootPath/android/bin/AndroidManifest.xml | rev | cut -d'"' -f1 | rev)

    #debug required
    if [ ! -z "$(echo $targets | grep d)" ]; then
        info "A bug? gdb on the way!"
        if ! ndk-gdb; then
            info "Note: this does not work with Android<2.3 and Cyanogen devices!" $orange
            error_and_quit "Could not start gdb. Did your build a debug version?"
        fi
    #launch required
    elif [ ! -z $(echo $1 | grep r) ]; then
        info "Running app '$gameName'..."

        if ! adb shell am start -n $packageName/$activityName; then
            error_and_quit "Could not run $gameName!"
        fi
    fi

    if [ $run_logcat = 1 ]; then
        info "Launching adb logcat..."
        adb logcat -c && adb logcat
    fi

    if [ $stack_trace = 1 ]; then
        info "Printing latest dump crashes"
        check_package_in_PATH "ndk-stack" "android-ndk"
        adb logcat | ndk-stack -sym $rootPath/android/libs/armeabi-v7a
    fi
}
