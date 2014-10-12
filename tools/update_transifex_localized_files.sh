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

[YOUR-PROJECT-RESOURCE (automatically set by tx)]
file_filter = android/res/values-<lang>/strings.xml
source_file = android/res/values/strings.xml
source_lang = en'
        exit 1
    fi

######### 1 : Pull files. #########
    temp=$(mktemp -d)
    cp -r android/res/. $temp
    find android/res -name 'strings.xml' -exec rm {} \;

    info "Pulling files..."
    if ! tx pull -s -a 1>/dev/null; then
        cp -r $temp/. android/res/
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
    cd android/res/

    for lang in values-*; do
      if grep -q '_' <<< $lang; then
        error_and_quit "Invalid name: $lang.
You must edit $rootPath/.tx/config and add it to the lang_map. It shoul looks like:
[main]
host = https://www.transifex.com
lang_map = fr_CA:fr-rCA,pt_BR:pt-rBR,pt_PT:pt,zh_CN:zh-rCN,zh_HK:zh-rHK,zh_TW:zh-rTW,da_DK:da-rDK,de_DE:de,tr_TR:tr,fr_FR:fr,es_ES:es,hu_HU:hu,sv_SE:sv-rSE,bg_BG:bg,el_GR:el,kn_IN:kn-rIN,cs_CZ:cs,gl_ES:gl_rES,ms_MY:ms_rMY
        "
    # Not needed anymore: it can be coded in the transifex .config file (see upon)
    #       suffix=r$(echo $lang | cut -d '_' -f 2 | tr [A-Z] [a-z])
    #       newName=$(echo $lang | cut -d '_' -f 1)-$suffix

    #       info "\t$lang is a special local.. renaming it ($lang -> $newName)" $orange

    #       mkdir $newName 2>/dev/null
    #       mv $lang/strings.xml $newName/
    #       rm -r $lang
      fi
    done

######### 3 : Change strings.xml content. Remove \ character when followed by an "'" and add extra "#########
    info "Removing useless escape char (sequence=\"\'\")"
    for file in $(find . -name strings.xml); do
        sed -i -e "s|\\\'|'|g" -e 's|">|">"|g' -e 's|</string>|"</string>|g' $file
    done

    info "Done. Cya!"
