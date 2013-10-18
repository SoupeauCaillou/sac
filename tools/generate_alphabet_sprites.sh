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
export SAC_USAGE="$0 fonts-directory output-directory"
export SAC_OPTIONS=""
export SAC_EXAMPLE="$0 $(cd $rootPath && pwd)/external_res/my_fonts/ $(cd $rootPath && pwd)/unprepared_assets/alphabet"

######### 0 : Check requirements. #########
    if [ $# != 2 ]; then
        error_and_usage_and_quit "Need the path to the fonts directory and the output directory"
    fi

    cd $fromWhereAmIBeingCalled

    if [ ! -d $1 ]; then
        error_and_quit "Fonts directory '$1' does not exist"
    elif [ ! -d $2 ]; then
        error_and_quit "Output directory '$2' does not exist"
    fi
    fonts=$(ls $1/*.ttf 2>/dev/null)
    output=$2

    if [ -z "$fonts" ]; then
        error_and_quit "No fonts (.ttf) found in $1"
    fi

######### 1 : Process. #########

size=60
suffix="_typo.png"

override=-1
function generate_sprite {
    result="Yes"

    # if destination sprite does already exist, ask the user for a confirmation
    if [ $override = -1 ] && [ -f $2$suffix ]; then
	   info "\nSprite $2$suffix('$1') already exists. Override it?" $orange
       select result in Yes No Yes-for-all No-for-all ; do
            if [ $result = "Yes-for-all" ]; then
                override=1
            elif [ $result = "No-for-all" ]; then
                override=0
            fi
            break
        done 
    fi

    if [ $override = 1 ] || [ "$result" = Yes ]; then
        for font in $fonts; do
            if ($whereAmI/does_TTFfont_contain_this_character.sh $font $1); then
                convert -background transparent -fill white -font $font -pointsize ${size} label:${1} PNG32:$output/${2}${suffix} &>/dev/null
                return $?
            fi
        done

        #if no font had the character, force quit
        echo
        error_and_quit "Character '$1' (hex=$2) could not be found in any .ttf file. 
        Please add a .ttf file containing it (use does_TTFfont_contain_this_character to find a matching font)"
    else
        info "Skipped $1" $orange
    fi

    # sprite was not generated, return success because this is user's choice
    return 0
}

function generate_for_list {
    printf "Treating... "
    for i in $@; do
        printf $i

        dec=$(printf "%x" "'$i")
        if ! generate_sprite $i $dec; then
            info "Error while generating character $i" $red
            printf "Treating... "
        fi
    done
    echo
}

info "Generating A->Z"
generate_for_list {A..Z} 

info "Generating a->z"
generate_for_list {a..z} 

info "Generating 0->9"
generate_for_list {0..9} 

info "Generating punctuation"
# ? needs to be escaped for an unknown reason...
ponct=", - . / : ; < = > \? _ ! \" # ' ( )"
generate_for_list $ponct

info "Generating accented letters"
specials=$(cat $rootPath/res/values*/strings.xml | tr -d '\\[:alnum:]'"$ponct" | grep -o . | sort -n | sed '$!N; /^\(.*\)\n\1$/!P; D' | tr '\n' ' ')
generate_for_list $specials 

info "Done! Do not forget to reexport atlas now and to run generate_alphabet_font_descriptor script!"

