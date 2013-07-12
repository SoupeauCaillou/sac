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
source $whereAmI/coolStuff.sh

export SAC_USAGE="$0"
export SAC_OPTIONS=""
export SAC_EXAMPLE="${green}$0"

if [ $# != 0 ]; then
    usage_and_quit
fi

    ############# STEP 0: verify env
    check_package_in_PATH "R" "r-base"

    ############# STEP 1: prepara datas and files
    datafile=$(mktemp)
    datafile_rscript=$(mktemp)
    count=$(echo $rootPath/assets/entities// | tr -c -d '/' | wc -c)
    for file in $(find $rootPath/assets/entities/ -name '*.entity'); do
        if (grep 'z =' $file -q); then
            info "$file contains a Z information, adding it..."
            echo $(echo $file | cut -d / -f $count-) $(grep 'z =' $file | cut -d '=' -f 2 | tr -d ' ') >> $datafile
        fi
    done
    output=/tmp/output.png
    rm $output 2>/dev/null

    ############# STEP 2: generate the linestack with R
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

