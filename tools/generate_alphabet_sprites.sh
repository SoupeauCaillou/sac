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

######### 1 : Process. #########

cd $fromWhereAmIBeingCalled/$1

typo=/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansCondensed.ttf
typo_mono=/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf

size=60
suffix="_typo.png"

override=-1
function generate_sprite {
    result=""

    # if destination sprite does already exist, ask the user for a confirmation
    if [ $override = -1 ] && [ -f ${2}${suffix} ]; then
	   info "Sprite ${2}${suffix}('$1') already exists. Override it?" $orange
       select result in Yes No Yes-for-all No-for-all; do
            if [ $result = "Yes-for-all" ]; then
                override=1
            elif [ $result = "No-for-all" ]; then
                override=0
            fi
            break
        done
    fi

    if [ $override = 1 ] || [ "$result" = Yes ]; then
        convert -background transparent -fill white -font $3 -pointsize ${size} label:${1} PNG32:${2}${suffix}
        return $?
    else
        info "Skipped $1" $orange
    fi

    # sprite was not generated, return success because this is user's choice
    return 0
}

info "Generating A->Z"
for i in {A..Z};
do
    dec=$(printf "%x" "'$i")
    generate_sprite $i $dec $typo
done

info "Generating a->z"
for i in {a..z};
do
    dec=$(printf "%x" "'$i")
    generate_sprite $i $dec $typo
done

info "Generating 0->9"
for i in {0..9};
do
    dec=$(printf "%x" "'$i")
    generate_sprite $i $dec $typo_mono
done

info "Generating punctuation"
ponct="! ? # ' ( ) , - . ; : ="
for i in $ponct; 
do
    dec=$(printf "%x" "'$i")
    generate_sprite $i $dec $typo
done

info "Generating accented letters"
specials=$(cat $rootPath/res/values*/strings.xml | tr -d "[:alnum:]$ponct" | grep -o . | sort -n | uniq | tr '\n' ' ')

printf "Treating... "
for i in $specials;
do
    printf $i

    dec=$(printf "%x" "'$i")

    if (!(generate_sprite $i $dec $typo &>/dev/null)); then
        info "Error while generating character $i" $red
        printf "Treating... "
    fi
done
echo 
info "Done! Do not forget to reexport atlas now and to run generate_alphabet_font_descriptor script!"

