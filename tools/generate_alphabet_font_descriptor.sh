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
source coolStuff.sh

#how to use the script
export SAC_USAGE="$0 alphabet-directory font-name: generate the font descriptor file in assets/ directory for the given font"
export SAC_OPTIONS=""
export SAC_EXAMPLE="$0 $(cd $rootPath && pwd)/unprepared_assets/alphabet myfont"

######### 0 : Check requirements. #########
    if [ $# != 2 ]; then
        error_and_usage_and_quit "Need the path to the alphabet directory and the font name"
    fi

    if [ ! -d "$fromWhereAmIBeingCalled/$1" ]; then
        error_and_usage_and_quit "Directory $1 does not exist!"
    fi

    if [ $(find $fromWhereAmIBeingCalled/$1 -name "*_$2.png" | wc -l) = 0 ]; then
        error_and_usage_and_quit "Could not find any *_$2.png image in $1!"
    fi

 
######### 1 : Process. #########
output=$rootPath/assets/$2.font
rm -f $output

function write {
    echo $@ >> $output
}

write "#FONT: $2" 
write "#char,width,height"

cd $fromWhereAmIBeingCalled/$1

info "Processing.."
for i in $(ls *_$2.png); do
	size=$(identify $i | cut -d\  -f 3)
	c=$(echo $i | cut -d_ -f1)
	width=$(echo $size | cut -dx -f1)
	height=$(echo $size | cut -dx -f2)
	write "$c=$width,$height"
done
info "Do not forget to generate the atlas too!" $orange
info "Done!"
