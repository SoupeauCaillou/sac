#!/bin/bash

#from where are we calling it
fromWhereAmIBeingCalled=$PWD
#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
cd $whereAmI
#if we executed a linked script; go to the real one
if [  -h $0 ]; then
    whereAmI+=/$(dirname $(readlink $0))
    cd $whereAmI
fi
rootPath=$whereAmI"/../.."
gameName=$(grep 'project(' $rootPath/CMakeLists.txt | cut -d '(' -f2 | tr -d ')')

#import cool stuff
source cool_stuff.sh

#get the list of available targets (need to be displayed in OPTIONS help
TARGETS_LIST=$(grep SUPPORTED_TARGETS $rootPath/sac/CMakeLists.txt | head -1 | cut -d " " -f 2- | tr -d ')')

#how to use the script
export SAC_USAGE="$0"
export SAC_OPTIONS=""
export SAC_EXAMPLE="$0: will download localized texts from transifex and automatically update android/res/values* directories"

######### 0 : Check requirements. #########
    if [ $# != 0 ]; then
        error_and_usage_and_quit "I don't need any argument!"
    fi
    check_package tx transifex-client

    cd $rootPath
    if ! tx status 1>/dev/null; then
        info "You didn't initialize transifex yet! Please run 'tx init' in $(cd $rootPath && pwd) then 'tx set --auto-remote <url>'" $red
        info "It should create a .tx/config file and you should modify it as:" $orange
        echo '[main]
host = https://www.transifex.com
lang_map = fr_CA:fr-rCA,pt_BR:pt-rBR,pt_PT:pt,zh_CN:zh-rCN,zh_HK:zh-rHK,zh_TW:zh-rTW

[YOUR-PROJECT-RESOURCE (automatically set by tx)]
file_filter = assets/strings/<lang>.txt
source_lang = en'
        exit 1
    fi

######### 1 : Pull files. #########
    temp=$(mktemp -d)
    mv assets/strings/* $temp

    info "Pulling files..."
    if ! tx pull -a -s -f; then
        mv $temp/* assets/strings
        error_and_quit "Your credidentials are wrong! File ~/.transifexrc should look like:
[https://www.transifex.com]
username = user
token =
password = p@ssw0rd
hostname = https://www.transifex.com
"
    else
        info "Removing old files..."
        rm -rf $temp
    fi
######### 2 : Rename some folders. #########
######### These are folders with a local like fr-BE, which must be renamed fr-rbe for Android ########
    cd assets/strings/

    for lang in *.txt; do
      if grep -q '_' <<< $lang; then
        error_and_quit "Invalid name: $lang.
You must edit $rootPath/.tx/config and add it to the lang_map. It shoul looks like:
[main]
host = https://www.transifex.com
lang_map = fr_CA:fr-rCA,pt_BR:pt-rBR,pt_PT:pt,zh_CN:zh-rCN,zh_HK:zh-rHK,zh_TW:zh-rTW
        "
      fi
    done

    info "Done. Cya!"
