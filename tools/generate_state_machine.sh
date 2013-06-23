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
    check_package dot graphfiz

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
#first arg: method name
#second arg: file name
#third arg: output file
get_return_within_method() {
    info "Treating file $2, looking for method '$1'..."
    method_line_start=$(grep -n "Scene::Enum $1(" $2 | cut -f1 -d:)

    lines=$(tail -n +$method_line_start $2)
    IFS=$'\n'

    opening_brace_count=0
    closing_brace_count=0
    for line in $lines; do
        #there is a specific case. FOR_EACH macros are using a '{' implicitely, so we need to count them too
        if (grep -q 'FOR_EACH' <<< $line); then
            # echo 'there is a foreach, special case opening_brace_count+=1 (assuming there is only once per line)'
            opening_brace_count=$(expr $opening_brace_count + 1)
        fi

        opening_brace_count=$(expr $opening_brace_count + $(echo $line | tr -c -d '{' | wc -c))
        closing_brace_count=$(expr $closing_brace_count + $(echo $line | tr -c -d '}' | wc -c))
        # echo "still in doupdate:" $line $opening_brace_count $closing_brace_count

        if (grep 'return ' -q <<< $line); then
            next_state=$(echo $line | sed -e 's/.*return //g' -e 's/Scene:://g')

            #if we are using return like 'return nextState(some params);', we have to check the nextState function!
            if (grep '(' -q <<< $next_state); then
                function_name=$(echo $next_state | cut -d'(' -f1)
                # echo "oops, need to check function $function_name too"
                get_return_within_method "$function_name" $2 $3
            else
                # echo "there is a return other here: next_state is $next_state"
                echo "$state -> $next_state" >> $3
            fi
        fi


        if [ $opening_brace_count = $closing_brace_count ]; then
            # echo "same count of braces, end of doupdate function"
            break;
        fi
    done
}

    info "todo: chercher le 'if' au dessus du return, afin de le mettre en condition de l'automate..." $red


    temp_file=$(mktemp)
    statesDirectory=$rootPath"/sources/states"

    #get the list of tates
    states=$(cd $statesDirectory && echo * | sed -e 's/.cpp//g' -e 's/Scenes.h//' -e 's/Scene$/ /g' -e 's/Scene / /g')
    echo "States are: $states"

    #there are some specifics states more: the fade out/in states. There are registered in sources/#GameName#Game.cpp
    gameName=$(cat $rootPath/CMakeLists.txt | grep 'project(' | cut -d '(' -f2 | tr -d ')')

    OLD_IFS=$IFS
    IFS=$'\n'
    for fade_state in $(grep 'registerState' $rootPath/sources/${gameName}Game.cpp | grep 'Scene::CreateFadeSceneHandler'); do
        fade_state_name=$(echo $fade_state | sed 's/.*(Scene:://' | cut -d ',' -f1)
        fade_state_next_state=$(echo $fade_state | cut -d ',' -f5 | cut -d ')' -f1 | sed 's/.*Scene:://')
        echo "$fade_state_name -> $fade_state_next_state" >> $temp_file
    done
    IFS=$OLD_IFS

    #for each state, get its DoUpdate function; and particulary all the 'return' inside it
    for state in $states; do
        # echo $state
        file=$statesDirectory/${state}Scene.cpp
        get_return_within_method 'update' $file $temp_file
    done

    #just for debug
    # cat $temp_file | sort -u

    output_file=/tmp/$gameName-state-machine.png

    #generate the graph using graphviz
    if (echo "digraph G { $(cat $temp_file | sort -u) }" | dot -Tpng > $output_file); then
        info "Successfully created $output_file"
        #open the eye of gnome to view the result
        eog $output_file
    else
        info "Error with graphfiz, could not create $output_file" $red
    fi

info "Good bye, my Lord!"
