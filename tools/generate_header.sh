#!/bin/bash

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
export SAC_USAGE="$0 [options]. Generate header for source files ('This file is part of...')"
export SAC_OPTIONS="\
-game(default): regenerate game's headers
\t-sac: regenerate sac's headers"
export SAC_EXAMPLE="$0: will regenerate headers for files located in platforms and sources directories"

######### 0 : Check requirements. #########
	if [ -z "$(pwd | grep /sac/tools)" ]; then
		error_and_quit "The script must be in sac/tools !"
	fi

######### 1 : Read arguments. #########
    GENERATE_SAC=0
    while [ "$1" != "" ]; do
        case $1 in
            "-sac")
                GENERATE_SAC=1
                ;;
            *)
                error_and_usage_and_quit "Unknown option '$1'"
        esac
        shift
    done

######### 2 : replace headers #########

sac_header='/*
    This file is part of GAME_NAME.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    GAME_NAME is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    GAME_NAME is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GAME_NAME.  If not, see <http://www.gnu.org/licenses/>.
*/'

if [ $GENERATE_SAC = 1 ]; then
    sac_header=${sac_header//GAME_NAME/Soupe Au Caillou}
    sac=$rootPath/sac
    files=$(find $sac/android $sac/api $sac/app $sac/base $sac/steering $sac/systems $sac/tests $sac/tools $sac/util -type f -not -path "*/libs/*")
else
    #replace GAME_NAME in ^ text with the real game name
    sac_header=${sac_header//GAME_NAME/$gameName}
    files=$(find $rootPath/sources $rootPath/platforms -type f)
fi

for file in $files; do
    ext=.$(basename $file | rev | cut -d'.' -f1 | rev). #points are relevant!

    #warning: there is still a bug, though, if $ext is like '.i' because it will
    #match with '.inl'; this is the reason why we force one dot each side
    if [[ ! ".cpp. .h. .hpp. .c. .inl. .java." =~ "$ext" ]]; then
        continue
    fi

    if grep -q 'This file is part of' $file; then
        line=$(grep -m 1 -n '\/\*' $file | cut -d ':' -f1)
        info "Replacing header in file $file"
        if [ "$line" = 1 ]; then
            endline=$(grep -m 1 -n '\*\/' $file | cut -d ':' -f1)

            sed -i "${line},${endline}d" $file

            echo "$sac_header" | cat - $file > tmp$PPID && mv tmp$PPID $file
        else
            info "$file: Header does not start on first line! Please put it on top line. Aborted" $red
        fi
    else
        info "Warning: $file does not contain header definition. Creating it..." $orange
        echo "$sac_header" | cat - $file > tmp$PPID && mv tmp$PPID $file
    fi
done


info "Good bye, my Lord!"
