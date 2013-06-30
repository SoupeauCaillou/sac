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
gameName=$(cat $rootPath/CMakeLists.txt | grep 'project(' | cut -d '(' -f2 | tr -d ')')

#import cool stuff
source coolStuff.sh
#how to use the script
export SAC_USAGE="$0 [options]. Generate header for source files (This file is part of...)"
export SAC_OPTIONS=""
export SAC_EXAMPLE="${green}TODO${default_color}"

######### 0 : Check requirements. #########
	if [ -z "$(pwd | grep /sac/tools)" ]; then
		error_and_quit "The script must be in sac/tools !"
	fi

######### 1 : Read arguments. #########
    while [ "$1" != "" ]; do
        case $1 in
            *)
                info "Unknow option '$1'"
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
*/
'
sac_header=${sac_header//GAME_NAME/$gameName}

for file in $(find $rootPath/sources $rootPath/platforms -type f); do

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
