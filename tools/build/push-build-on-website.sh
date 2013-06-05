#!/bin/bash

#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
cd $whereAmI
#if we executed a linked script; go to the real one
if [  -h $0 ]; then
	whereAmI+=/$(dirname $(readlink $0))
	cd $whereAmI
fi

#import cool stuff
source ../coolStuff.sh

#how to use the script
export USAGE="\
Copy an emscripten build (game.html and game.data) on a distant server, will the following syntax: gameName-currentDate
\t$0 -d build-directory -[specifics_options] [and their values]"
export OPTIONS="\
-h|--help: show this help
\t-d: ${red}[REQUIRED]${yellow}specify the build directory to copy.
\t-p: view the result in iceweasel.
\t-s: specify user/server address. Default value is 'soupeaucaillou@soupeaucaillou.com/prototypes'"
export EXAMPLE="\
${green}'$0 ../../../build/emscripten-release'${yellow} will copy the directory \
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
    while [ "$1" != "" ]; do
        case $1 in
            "-h" | "--help")
                usage_and_quit
                ;;
    	    "-d")
        		shift
        		DIRECTORY=$1
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

######### 2 : Configure variables #########
    gameName=$(cat $rootPath/CMakeLists.txt | grep 'project(' | cut -d '(' -f2 | tr -d ')')

    if (!( grep -q '@' <<< "$USER_SERVER" )); then
        error_and_quit "USER_SERVER couple $USER_SERVER does not contain '@' character!"
    fi
    USER=$(echo $USER_SERVER | cut -d '@' -f1)
    SERVER=$(echo $USER_SERVER | cut -d '@' -f2 | cut -d '/' -f1)

    if [ -z "$DIRECTORY" ] || [ ! -e "$DIRECTORY" ]; then
	error_and_quit "Directory '$DIRECTORY' does not exist"
    fi
    DIRECTORY_FORMATTED=$(echo $USER_SERVER | cut -d '/' -f2-)/$(echo $gameName-$(date +"%Y-%m-%d-%H-%M"))

######### 4 :  check if the necessary files are in folder.. #########
    HTML_FILE=$(find $DIRECTORY -maxdepth 1 -name '*.html')
    if [ -z "$HTML_FILE" ]; then
        error_and_quit "Could not locate any .html file in $DIRECTORY"
    fi
    DATA_FILE=$(find $DIRECTORY -maxdepth 1 -name '*.data')
    if [ -z "$DATA_FILE" ]; then
        error_and_quit "Could not locate any .data file in $DIRECTORY"
    fi
######### 5 : Push content on website. #########
    TMP_DIR=/tmp/$PPID
    mkdir $TMP_DIR
    cp $HTML_FILE $DATA_FILE $TMP_DIR
    lftp -u $USER $SERVER -e "mirror -R $TMP_DIR $DIRECTORY_FORMATTED && exit"
    rm -rf $TMP_DIR
######### 6 : (optional) Open navigator. #########
    if [ "$LAUNCH_ON_END" = "1" ]; then
        iceweasel $SERVER/$DIRECTORY_FORMATTED/
    fi
