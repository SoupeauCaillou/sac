#how to use the script
export PLATFORM_OPTIONS="\
-p [parameters]: use push-build-on-website script with 'parameters' to push on a distant server a build"

parse_arguments() {
    :
    PUSH_ARGS=""
    while [ "$1" != "" ]; do
        case $1 in
            "-p")
                shift
                PUSH_ARGS=$1
                ;;
        esac
        shift
    done
}

check_necessary() {
    :
}

compilation_before() {
    :
}

compilation_after() {
    :
}

launch_the_application() {
    #push on site required
    if [ ! -z "$(echo $TARGETS | grep p)" ]; then
        info "Pushing content on site..."
        ../../sac/tools/build/push-build-on-website.sh -d . $PUSH_ARGS
    fi

    #launch required
    if [ ! -z "$(echo $TARGETS | grep r)" ]; then
        info "Finding a navigator..."
        #find a navigator...
        navigator=""
        if ( type iceweasel 1>2 &2>/dev/null ); then
            info "iceweasel will be used"
            navigator="iceweasel"
        elif (type chromium 1>2 &2>/dev/null ) ;then
            info "iceweasel will be used"
            navigator="iceweasel"
        else
            info "can't find any navigator to view the result!" $red
        fi

        if [ ! -z $navigator ]; then
            info "Launch game in ${navigator}."
            $navigator $gameName.html
        fi
    fi
}


















