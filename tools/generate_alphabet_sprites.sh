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
export SAC_USAGE="$0 fonts-directory output-directory [option]"
export SAC_OPTIONS="\
-c|-check: test if every symbols are already in atlas\
-i|-inputs: inputs dir to search symbols"
export SAC_EXAMPLE="$0 $(cd $rootPath && pwd)/external_res/my_fonts/ $(cd $rootPath && pwd)/unprepared_assets/alphabet"

######### 0 : Check requirements. #########
    if [ $# -lt 2 ]; then
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

    shift #first arg is font directory
    shift #second arg is output directory

    preview_mode=0
    src_dir=''
    while [ "$1" != "" ]; do
        case $1 in
            "-c" | "-check")
                info "Preview mode"
                preview_mode=1
                ;;
            "-i" | "-inputs")
                shift
                i=0
                for arg in $@ 
                do
                    if [ $arg = "-c" -o $arg = "--check" ]; then
                        break
                    fi
                    src_dir[$i]="$PWD/$arg";
                    i=$((i+1));
                done
                echo ${src_dir[*]}
                ;;
            *)
                echo "Unknown option $1"
        esac
        shift
    done

######### 1 : Process. #########

size=60
suffix="_typo.png"

override=-1

# arg 1 is the symbol
# arg 2 is the hexa value of the symbol (arg 1)
# Generate the symbol and warn the user if it already exists. Return an error if no font contains the symbol
function generate_sprite {
    result="No"

    # if destination sprite does already exist, ask the user for a confirmation
    if [ -f $output/$2$suffix ]; then
        if [ $override = -1 ]; then
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
    else
        result="Yes"
    fi

    if [ $override = 1 ] || [ "$result" = Yes ]; then
        for font in $fonts; do
            if ($whereAmI/does_TTFfont_contain_this_character.sh $font $1); then
                convert -background transparent -fill white -font $font -pointsize ${size} label:${1} PNG32:$output/$2$suffix &>/dev/null
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

# arg 1 is the hexa value of the symbol
# verify that the symbol's sprite already exist. Return an error if it does not exist
function check_exist {
    if [ -f $output/$1$suffix ]; then
        return 0
    else
        return 1
    fi
}

#number of missing symbols (check mode only)
missing_symbols_count=0

# args are the list of symbols to generate
function iterate_through_list {
    printf "Treating... "
    for i in $@; do
        printf $i

        dec=$(printf "%x" "'$i")

        if [ "$preview_mode" = 1 ]; then
            if ! check_exist $dec; then
                echo
                missing_symbols_count=$(($missing_symbols_count + 1))
                info "$i(hex=$dec) sprite is not present in output folder" $red
            fi
        else
            if ! generate_sprite $i $dec; then
                info "Error while generating character $i" $red
                printf "Treating... "
            fi
        fi
    done
    echo
}

info "Symbols A->Z..."
iterate_through_list {A..Z} 

info "Symbols a->z..."
iterate_through_list {a..z} 

info "Symbols 0->9..."
iterate_through_list {0..9} 

info "Symbols punctuation..."
ponct=", - . / : ; < = > ? _ ! \" # ' ( )"
iterate_through_list $ponct

info "Symbols missing..."
if [ ${#src_dir[@]} -gt 0 ]; then
    for dir in $src_dir ; do
        info "Check in $dir ..."
        specials=$(cat ${dir%/}/values*/strings.xml | tr -d '\\[:alnum:]'"$ponct" | grep -o . | sort -d | perl -ne 'print unless $seen{$_}++' | tr '\n' ' ')
        iterate_through_list $specials 
    done
else
    specials=$(cat $rootPath/res/values*/strings.xml | tr -d '\\[:alnum:]'"$ponct" | grep -o . | sort -d | perl -ne 'print unless $seen{$_}++' | tr '\n' ' ')
    iterate_through_list $specials 
fi


if [ "$preview_mode" = 1 ]; then
    info "Done! There is/are $missing_symbols_count missing symbols!"
else
    info "Done! Do not forget to reexport atlas now and to run generate_alphabet_font_descriptor script!"
fi

