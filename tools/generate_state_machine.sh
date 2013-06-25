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
    gameName=$(cat $rootPath/CMakeLists.txt | grep 'project(' | cut -d '(' -f2 | tr -d ')')
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
        #if this is a comment line, skip it
        #Note: if this is a multiline comment (/* */ or #if 0 #endif), that won't work... but nvm
        if [ "$(echo $line | tr -d ' ' | cut -c1-2)" = "//" ]; then
            # echo "this is a comment!" $line
            continue
        fi

        opening_brace_count=$(expr $opening_brace_count + $(echo $line | tr -c -d '{' | wc -c))
        closing_brace_count=$(expr $closing_brace_count + $(echo $line | tr -c -d '}' | wc -c))
        # echo "still in doupdate:" $line $opening_brace_count $closing_brace_count

        if (grep 'return ' -q <<< $line); then
            next_state=$(echo $line | sed -e 's/.*return //g' -e 's/Scene:://g' | tr -d ';')

            #if we are using return like 'return nextState(some params);', we have to check the nextState function!
            if (grep '(' -q <<< $next_state); then
                function_name=$(echo $next_state | cut -d'(' -f1)
                # echo "oops, need to check function $function_name too"
                get_return_within_method "$function_name" $2 $3
            else
                # echo "there is a return other here: next_state is $next_state"
                if [ "$state" != "$next_state" ]; then
                    echo "$state -> $next_state;" >> $3
                    if ! [[ $states =~ $next_state ]] && ! [[ $fade_states =~ $next_state ]]; then
                        # echo "$next_state not in states"
                        echo "$next_state [fillcolor=$unknown_state_color] " >> $3
                    fi
                fi
            fi
        fi


        if [ $opening_brace_count = $closing_brace_count ]; then
            # echo "same count of braces, end of doupdate function"
            break;
        fi
    done
}

    info "todo: chercher le 'if' au dessus du return, afin de le mettre en condition de l'automate..." $red

    #a list of colors is available here: http://www.graphviz.org/doc/info/colors.html
    fade_state_color="salmon2"
    unknown_state_color="darkgoldenrod1"
    initial_state_color="chartreuse3"


    temp_file=$(mktemp)
    echo "digraph G {
        node [ style = filled ]" > $temp_file

    statesDirectory=$rootPath"/sources/states"

    #get the list of states
    initial_states=$(grep 'sceneStateMachine.setup(' $rootPath/sources/${gameName}Game.cpp | cut -d ':' -f3 | cut -d ')' -f1)
    for initial_state in $initial_states; do
        echo "$initial_state [ fillcolor=$initial_state_color];" >> $temp_file
    done

    states=$(cd $statesDirectory && echo * | tr ' ' '\n' | sed -e 's/Scenes.h//' -e 's/\(.*\)\..*/\1/g' -e 's/Scene$/ /g' -e 's/Scene / /g' | tr '\n' ' ' | tr -s ' ')
    echo "States are: $states. Initial state(s) is(are) '"$initial_states"'"

    fade_states=""
    #there are some specifics states more: the fade out/in states. There are registered in sources/#GameName#Game.cpp
    OLD_IFS=$IFS
    IFS=$'\n'
    for fade_state in $(grep 'registerState' $rootPath/sources/${gameName}Game.cpp | grep 'Scene::CreateFadeSceneHandler'); do
        if $(echo $fade_state | cut -d '.' -f1 | grep '//' -q); then
            echo "this is a commented state! $fade_state"
            continue
        fi
        fade_states+="$fade_state "

        fade_state_name=$(echo $fade_state | sed 's/.*(Scene:://' | cut -d ',' -f1)
        fade_state_next_state=$(echo $fade_state | cut -d ',' -f5 | cut -d ')' -f1 | sed 's/.*Scene:://')

        echo "$fade_state_name [ fillcolor=$fade_state_color ];" >> $temp_file
        echo "$fade_state_name -> $fade_state_next_state;" >> $temp_file
    done
    IFS=$OLD_IFS

    #for each state, get its DoUpdate function; and particulary all the 'return' inside it
    for state in $states; do
        # echo $state
        file=$(find $statesDirectory -name "${state}*")
        get_return_within_method 'update' $file $temp_file
    done

    #just for debug
    # cat $temp_file | sort -u

    output_file=/tmp/$gameName-state-machine.png

    echo "}" >> $temp_file

    #generate the graph using graphviz but first ensure there is not twice the same line
    if (awk ' !x[$0]++' $temp_file | dot -Tpng > $output_file); then
        info "Successfully created $output_file"
        #open the eye of gnome to view the result
        eog $output_file
    else
        info "Error with graphfiz, could not create $output_file" $red
    fi

info "Good bye, my Lord!"
