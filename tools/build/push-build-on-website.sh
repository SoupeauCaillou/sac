#!/bin/bash

#where we are right now
whereUserIs=$(pwd)

#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
cd $whereAmI
#if we executed a linked script; go to the real one
if [  -h $0 ]; then
	whereAmI+=/$(dirname $(readlink $0))
	cd $whereAmI
fi

#import cool stuff
source ../cool_stuff.sh

#how to use the script
export SAC_USAGE="\
Copy an emscripten build (game.html and game.data) on a distant server, will the following syntax: gameName-branchName-commitId-currentDate
\t$0 -d build-directory -[specifics_options] [and their values]"
export SAC_OPTIONS="\
-h|-help: show this help
\t-d: ${red}[REQUIRED]${default_color}specify the build directory to copy.
\t-c: create an empty index.html file at the server root directory. ${red}[Warning: will overwrite any existing index.html!]${yellow}
\t-p: view the result in iceweasel.
\t-s: specify user/server address. Default value is 'soupeaucaillou@soupeaucaillou.com/prototypes'"
export SAC_EXAMPLE="\
${green}'$0 ../../../build/emscripten-release'${default_color} will copy the directory \
on server, with format {gameName}-{date}"

######### 0 : Check requirements. #########
	if [ -z "$(pwd | grep /sac/tools/build)" ]; then
		error_and_quit "The script must be in sac/tools/build !"
	fi

######### 1 : Read arguments. #########
	if [ $# = 0 ]; then
		usage_and_quit
    fi

    USER_SERVER='soupeaucaillou@soupeaucaillou.com/prototypes'
    DIRECTORY=''
    LAUNCH_ON_END=0
    CREATE_INDEX=0
    INTERACTIVE_MODE=0
    while [ "$1" != "" ]; do
        case $1 in
            "-h" | "--help")
                usage_and_quit
                ;;
    	    "-d")
        		shift
        		DIRECTORY=${whereUserIs}/${1}
        		;;
            "-c")
                CREATE_INDEX=1
                ;;
            "-i")
                INTERACTIVE_MODE=1
                ;;
            "-p")
                LAUNCH_ON_END=1
                ;;
            "-s")
                shift
                USER_SERVER=$1
                ;;
            -*)
                error_and_quit "Unknown option: $1"
                ;;
            *)
                error_and_quit "What is '$1'?! Not handled."
        esac
        shift
    done

    rootPath=$whereAmI"/../../.."

    if [ $INTERACTIVE_MODE = 1 ]; then
        info "INTERACTIVE_MODE not yet available. Abort" $red
        exit
    fi
######### 2 : Configure variables #########
    info "Setting variables..."

    gameName=$(grep 'project(' $rootPath/CMakeLists.txt | cut -d '(' -f2 | tr -d ')')

    if (!( grep -q '@' <<< "$USER_SERVER" )); then
        error_and_quit "USER_SERVER couple $USER_SERVER does not contain '@' character!"
    fi
    USER=$(echo $USER_SERVER | cut -d '@' -f1)
    SERVER=$(echo $USER_SERVER | cut -d '@' -f2 | cut -d '/' -f1)

    if [ -z "$DIRECTORY" ] || [ ! -e "$DIRECTORY" ]; then
	   error_and_quit "Directory '$DIRECTORY' does not exist"
    fi
    DATE=$(date +"%Y.%m.%d.%H.%M")
    GIT_BRANCH=$(cd $rootPath && git rev-parse --abbrev-ref HEAD)
    GIT_COMMIT=$(cd $rootPath && git rev-parse --short HEAD)
    DIRECTORY_FORMATTED=$(echo $USER_SERVER | cut -d '/' -f2-)$(echo $gameName-$GIT_BRANCH-$GIT_COMMIT-$DATE)
######### 4 :  check if the necessary files are in folder.. #########
    info "Checking if $DIRECTORY contains a .html file and a .data file..."
    HTML_FILE=$(find $DIRECTORY -maxdepth 1 -name '*.html' -printf "%f\n")
    if [ -z "$HTML_FILE" ]; then
        error_and_quit "Could not locate any .html file in $DIRECTORY. Are you sure you've done a build?"
    fi
    DATA_FILE=$(find $DIRECTORY -maxdepth 1 -name '*.data' -printf "%f\n")
    if [ -z "$DATA_FILE" ]; then
        error_and_quit "Could not locate any .data file in $DIRECTORY. Are you sure you've done a build?"
    fi

######### 5 : Prepare the datas #########
    TMP_DIR=$(mktemp -d)
    cp $DIRECTORY/$HTML_FILE $DIRECTORY/$DATA_FILE $TMP_DIR

    #create a simple index.html which just does a redirection...
    if [ "$CREATE_INDEX" = "1" ]; then
        TMP_FILE=$(mktemp)
        echo "<html><head><title>Redirection...</title>
        <meta http-equiv=\"refresh\" content=\"0; URL=$(echo $gameName-$GIT_BRANCH-$GIT_COMMIT-$DATE)/$HTML_FILE\">
        </head><body></body></html>" > $TMP_FILE
    fi

######### 6 : Prepare the request. #########
    REQUEST_LFTP="mirror -R $TMP_DIR $DIRECTORY_FORMATTED"
    if [ "$CREATE_INDEX" = "1" ]; then
        REQUEST_LFTP=$REQUEST_LFTP" && put $TMP_FILE -o $DIRECTORY_FORMATTED/../index.html"
    fi
    REQUEST_LFTP=$REQUEST_LFTP" && exit"

######### 7 : Push content on website. #########
    info "User is '$USER' on server '$SERVER'. The directory name will be '$DIRECTORY_FORMATTED'. Asking for ftp password..."
    lftp -u $USER $SERVER -e "$REQUEST_LFTP"

######### 8 : (optional) Open navigator. #########

    if [ "$LAUNCH_ON_END" = "1" ]; then
        info "Launching iceweasel for page $SERVER/$DIRECTORY_FORMATTED/$HTML_FILE..."
        iceweasel $SERVER/$DIRECTORY_FORMATTED/$HTML_FILE &
    fi

info "Good bye, my Lord!"
