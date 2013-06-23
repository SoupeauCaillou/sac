#!/bin/bash

#where we are right now
whereUserIs=$(pwd)

#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
cd $whereAmI
#if we executed a linked script; go to the real one
if [  -h $0 ]; then
	whereAmI+=/$(dirname $(readlink $0))
	cd $whereAmI
fi

#import cool stuff
source coolStuff.sh

#how to use the script
export SAC_USAGE="\
Generate a png file from your state machine"
export SAC_OPTIONS="\
"
export SAC_EXAMPLE="\
"

######### 0 : Check requirements. #########
	if [ -z "$(pwd | grep /sac/tools)" ]; then
		error_and_quit "The script must be in sac/tools and not in $(pwd)!"
	fi

######### 1 : Read arguments. #########
    while [ "$1" != "" ]; do
        case $1 in
            -*)
                error_and_quit "Unknown option: $1"
                ;;
            *)
                error_and_quit "What is '$1'?! Not handled."
        esac
        shift
    done

    rootPath=$whereAmI"/../.."
######### 2 :
    temp_file=$(mktemp)
    statesDirectory=$rootPath"/sources/states"
    states=$(cd $statesDirectory && echo * | sed -e 's/.cpp//g' -e 's/Scenes.h//' -e 's/Scene / /g')
    echo "States are: $states"

    for state in $states; do
        # echo $state
        file=$statesDirectory/${state}Scene.cpp
        next_states_line=$(grep -n 'return Scene::' $file | cut -f1 -d:)
        for next_state_line in $next_states_line; do
            next_state=$(sed -n "${next_state_line}p" $file | sed -e 's/return Scene:://' | tr -d '; ')
            echo "$state -> $next_state" >> $temp_file
            #todo: chercher le 'if' au dessus du return, afin de le mettre en condition de l'automate...
        done
    done
    cat $temp_file | uniq

    output_file=/tmp/sac_graphfiz.png

    if (echo "digraph G { $(cat $temp_file | uniq) }" | dot -Tpng > $output_file); then
        info "Successfully created $output_file"
        eog $output_file
    else
        info "Error with graphfiz, could not create $output_file" $red
    fi

info "Good bye, Lord!"
