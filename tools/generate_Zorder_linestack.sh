#!/bin/bash

#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
#if we executed a linked script; go to the real one
if [ -h $0 ]; then
    whereAmI+=/$(dirname $(readlink $0))
fi
rootPath=$whereAmI"/../.."
gameName=$(cat $rootPath/CMakeLists.txt | grep 'project(' | cut -d '(' -f2 | tr -d ')')

#import cool stuff
source $whereAmI/cool_stuff.sh

export SAC_USAGE="$0"
export SAC_OPTIONS=""
export SAC_EXAMPLE="${green}$0"

if [ $# != 0 ]; then
    usage_and_quit
fi

    ############# STEP 0: verify env
    check_package_in_PATH "R" "packages r-base and r-cran-vegan"
    check_package "bc"

    ############# STEP 1: prepare data and files
    datafile=$(mktemp)
    datafile_rscript=$(mktemp)

    count=$(echo $rootPath/assets/entities// | tr -c -d '/' | wc -c)
    for file in $(find $rootPath/assets/entities/ -name '*.entity'); do
        if (grep 'z =' $file -q); then
            info "$file contains a Z information, adding it..."

            #retrieve entity_name (path from assets/entities/ root)
            entity_name=$(echo $file | cut -d / -f $count-)

            #if entity has an 'Anchor' component, then its z is relative to its parent
            z=$(grep 'z =' $file | cut -d '=' -f 2 | tr -d ' ')
            
            if (grep '\[Anchor\]' $file -q); then
                parent=$(grep 'parent%name' $file | cut -d '=' -f 2 | tr -d ' ')

                if [ -z "$parent" ]; then
                    info "$entity_name: Oops! Found an entity with an anchor but no parent... setting its z to -1" $orange

                    z=$(echo $z - 1 | bc)
                else
                    #since we dont know if its parent has already been parsed, we store it in file with @parent 
                    z=$z@$parent
                fi
            fi

            echo $entity_name $z >> $datafile
        fi
    done
    output=/tmp/output.png
    rm $output 2>/dev/null

    ############# STEP 2: handle Anchor issue (calculate absolute Z)
    while (grep '@' $datafile -q); do
        line=$(grep '@' $datafile -m 1)

        # echo $line

        my_name=$(echo $line | cut -d ' ' -f 1)

        # here rel_z contains value@parent_name
        rel_z=$(echo $line | cut -d ' ' -f 2)
        
        parent_name=$(echo $rel_z | cut -d '@' -f 2)
        rel_z=$(echo $rel_z | cut -d '@' -f 1)

        parent_z=$(grep '$parent_name ' $datafile | cut -d ' ' -f 2)

        if [ -z "$parent_z" ]; then
            info "$my_name: Oops! $parent_name is parent of $my_name but couldn't find it. Setting z to -1" $orange
            parent_z=-1
        elif [[ "$parent_z" =~ "@" ]]; then
            info "$my_name: Oops! $parent_name is parent of $my_name but it has a parent too! Too complicated" $orange
            parent_z=-1
        fi
        

        abs_z=$(echo $rel_z + $parent_z | bc)

        sed -i "s|$line|$my_name $abs_z|" $datafile
    done

    ############# STEP 3: generate the linestack with R
    #couldn't pipe the result directly, so using a temporary file...
    info "Executing R script..."
echo "\
#!/usr/bin/env Rscript
library(vegan)
png('$output')
datas<-read.table('$datafile',sep='')
linestack(datas[,2],datas[,1],side='right', air=1.5)
linestack(unique(datas[,2]),unique(datas[,2]),side='left', add=TRUE, air=1.5)
title(main='Entities Z order')" > $datafile_rscript

    #and then executing the script
    chmod +x $datafile_rscript
    $datafile_rscript

    rm $datafile_rscript $datafile

    #final visualization
    info "Let's take a look at the result!"
    eog $output

info "Cya!"



#!/bin/bash

