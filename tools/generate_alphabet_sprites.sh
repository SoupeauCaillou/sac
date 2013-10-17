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
export SAC_USAGE="$0 alphabet-directory"
export SAC_OPTIONS=""
export SAC_EXAMPLE="$0 $(cd $rootPath && pwd)/unprepared_assets/alphabet"

######### 0 : Check requirements. #########
    if [ $# != 1 ]; then
        error_and_usage_and_quit "Need the path to the alphabet directory"
    fi

######### 1 : Read arguments. #########
    CHECK_GENERATED_SPRITES=0
    while [ "$1" != "" ]; do
        case $1 in
            "-h" | "-help")
                usage_and_quit
                ;;
            *)
                alphabet=$1
        esac
        shift
    done

######### 2 : Process. #########

cd $fromWhereAmIBeingCalled/$alphabet

typo=/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansCondensed.ttf
typo_mono=/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf
# Find fonts for a language: 'fc-list :lang=zh' (for chinese)
typo_asian=/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf

size=60
suffix="_typo.png"

override=-1
function generate_sprite {
    result="Yes"

    # if destination sprite does already exist, ask the user for a confirmation
    if [ $override = -1 ] && [ -f ${2}${suffix} ]; then
	   info "\nSprite ${2}${suffix}('$1') already exists. Override it?" $orange
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
        convert -background transparent -fill white -font $3 -pointsize ${size} label:${1} PNG32:${2}${suffix} &>/dev/null
        return $?
    else
        info "Skipped $1" $orange
    fi

    # sprite was not generated, return success because this is user's choice
    return 0
}

function generate_for_list {
    printf "Treating... "
    typo=$1
    shift
    for i in $@; do
        printf $i

        dec=$(printf "%x" "'$i")
        if ! generate_sprite $i $dec $typo; then
            info "Error while generating character $i" $red
            printf "Treating... "
        fi
    done
    echo
}

info "Generating A->Z"
generate_for_list $typo {A..Z} 

info "Generating a->z"
generate_for_list $typo {a..z} 

info "Generating 0->9"
generate_for_list $typo_mono {0..9} 

info "Generating punctuation"
ponct="! ? # ' ( ) , \- . ; : = < > _ "
generate_for_list $typo $ponct 

info "Generating Asian characters"
asian_chars=$(cat $rootPath/res/values-ja/strings.xml $rootPath/res/values-zh/strings.xml | tr -d '\\[:alnum:]'"$ponct" | grep -o . | sort -n | uniq | tr '\n' ' ')
#not working well yet
generate_for_list $typo_asian $asian_chars 

info "Generating accented letters"
specials=$(cat $rootPath/res/values*/strings.xml | tr -d '\\[:alnum:]'"$ponct$asian_chars" | grep -o . | sort -n | uniq | tr '\n' ' ')
generate_for_list $typo $specials 

info "Done! Do not forget to reexport atlas now and to run generate_alphabet_font_descriptor script!"

