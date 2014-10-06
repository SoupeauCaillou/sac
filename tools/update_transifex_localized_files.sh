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
gameName=$(cat $rootPath/CMakeLists.txt | grep 'project(' | cut -d '(' -f2 | tr -d ')')

#import cool stuff
source cool_stuff.sh

#get the list of available targets (need to be displayed in OPTIONS help
TARGETS_LIST=$(grep SUPPORTED_TARGETS ../CMakeLists.txt | head -1 | cut -d " " -f 2- | tr -d ')')

#how to use the script
export SAC_USAGE="$0"
export SAC_OPTIONS=""
export SAC_EXAMPLE="$0: will download localized texts from transifex and automatically update res/values* directories"

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
file_filter = res/values-<lang>/strings.xml
source_file = res/values/strings.xml
source_lang = en'
		exit 1
	fi

######### 1 : Pull files. #########
	temp=$(mktemp -d)
	cp -r res/. $temp
	find res -name 'strings.xml' -exec rm {} \;

    info "Pulling files..."
	if ! tx pull -s -a 1>/dev/null; then
		cp -r $temp/. res/
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
	info "Renaming values* folder (if needed)"
	cd res/

	for lang in values-*; do
		if grep -q '_' <<< $lang; then
			suffix=r$(echo $lang | cut -d '_' -f 2 | tr [A-Z] [a-z])
			newName=$(echo $lang | cut -d '_' -f 1)-$suffix

			info "\t$lang is a special local.. renaming it ($lang -> $newName)" $orange

			mkdir $newName 2>/dev/null
			mv $lang/strings.xml $newName/
			rm -r $lang
		fi
	done

######### 3 : Change strings.xml content. Remove \ character when followed by an "'" and add extra "#########
	info "Removing useless escape char (sequence=\"\'\")"
	for file in $(find . -name strings.xml); do
		sed -i -e "s|\\\'|'|g" -e 's|">|">"|g' -e 's|</string>|"</string>|g' $file
	done

	info "Done. Cya!"
