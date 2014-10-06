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

#how to use the script
export SAC_USAGE="$0 alphabet-directory font-name: generate the font descriptor file in assets/ directory for the given font"
export SAC_OPTIONS=""
export SAC_EXAMPLE="$0 $(cd $rootPath && pwd)/unprepared_assets/alphabet typo"

######### 0 : Check requirements. #########
    if [ $# != 2 ]; then
        error_and_usage_and_quit "Need the path to the alphabet directory"
    fi

    if [ ! -d "$fromWhereAmIBeingCalled/$1" ]; then
        error_and_usage_and_quit "Directory $1 does not exist!"
    fi

    if [ $(find $fromWhereAmIBeingCalled/$1 -name "*_typo.png" | wc -l) = 0 ]; then
        error_and_usage_and_quit "Could not find any *_typo.png image in $1!"
    fi


######### 1 : Process. #########
output=$rootPath/assets/typo.font
rm -f $output

function write {
    echo $@ >> $output
}

write "#FONT: typo"
write "#char,width,height"

cd $fromWhereAmIBeingCalled/$1

info "Processing.."
for i in $(ls *_typo.png); do
	size=$(identify $i | cut -d\  -f 3)
	c=$(echo $i | cut -d_ -f1)
	width=$(echo $size | cut -dx -f1)
	height=$(echo $size | cut -dx -f2)
	write "$c=$width,$height"
done
info "Do not forget to generate the atlas too!" $orange
info "Done!"
