#how to use the script
export PLATFORM_OPTIONS=""

parse_arguments() {
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
    info "You can't compile on Windows version (yet?), so make will fail. That's ok, please open the project in Visual Studio!" $orange
}

compilation_after() {
    :
}

launch_the_application() {
    info "Not Handled yet!"
}


















