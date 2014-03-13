#how to use the script
export PLATFORM_OPTIONS="\
\tn: simply compile
\tr: run the app\t\t\t\t\t(options available, see below)
\td: run&debug the app with cgdb
\tl: use colorlog.sh script for colored logs\t(options available, see below)
You can also specify arguments:
\t--l|--log \"arguments for coloredlog script\": options for this script. See it for arguments availables
\t--r|--run \"arguments for game\": arguments handled by the game (--restore, --verbose, ..., whatever you did!)"

parse_arguments() {
    RUN_ARGS=""
    COLOREDLOGS_ARGS=""
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

            "--l" | "--log")
                shift
                TARGETS=$TARGETS"l"
                COLOREDLOGS_ARGS=$1
                ;;
            "--r" | "--run")
                shift
                TARGETS=$TARGETS"r"
                RUN_ARGS=$1
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
    :
}

init() {
    :
}

compilation_before() {
    :
}

compilation_after() {
    :
}

launch_the_application() {
    executable="./$gameName $RUN_ARGS"
    #debug required
    if [ ! -z "$(echo $TARGETS | grep d)" ]; then
        info "A bug? Cgdb on the way!"
        check_package cgdb
        cgdb -ex run $executable
    #launch required
    elif [ ! -z "$(echo $TARGETS | grep r)" ]; then
        #verbose required
        if [ ! -z "$(echo $TARGETS | grep l)" ]; then
            info "Launch with colored log."

            #coloredlogs shouldn't be empty
            if [ -z "$COLOREDLOGS_ARGS" ]; then
                info "No arg for color script ?\nChoose tag with option -l" $red
                info "I will use 'all' tag here"
                COLOREDLOGS_ARGS='all'
                sleep 3
            fi
            $executable | $rootPath/sac/tools/build/linux-coloredLogs.sh $COLOREDLOGS_ARGS

        else
            info "Launch game."
            $executable
        fi
    fi
}


















