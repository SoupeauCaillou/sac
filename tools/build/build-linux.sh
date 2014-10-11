#how to use the script
export PLATFORM_OPTIONS="\
\tn: simply compile
\tr: run the app\t\t\t\t\t(options available, see below)
\td: run&debug the app with cgdb
You can also specify arguments:
\t--r|--run \"arguments for game\": arguments handled by the game (--restore, --verbose, ..., whatever you did!)"

parse_arguments() {
    ret=0
    run_args=""
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

            "--r" | "--run")
                shift
                targets=$targets"r"
                run_args=$1
                ;;
            --*)
                info "Unknown option '$1', ignoring it and its arg '$2'" $red
                ret=$(($ret + 1))
                shift
                ;;
            -*)
                info "Unknown option '$1', ignoring it" $red
                ret=$(($ret + 1))
                ;;
        esac
        shift
    done
    return $ret
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
    executable="./$gameName $run_args"
    #debug required
    if [ ! -z "$(echo $targets | grep d)" ]; then
        info "A bug? Cgdb on the way!"
        check_package cgdb
        cgdb -ex run $executable
    #launch required
    elif [ ! -z "$(echo $targets | grep r)" ]; then
        #verbose required
        info "Launch game."
        $executable
    fi
}


















