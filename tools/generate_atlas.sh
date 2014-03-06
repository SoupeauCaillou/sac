#!/bin/bash

# path need :
# location of texture_packer (your_build_dir/sac/build/cmake)
# location of PVRTexToolCL (need to install it from http://www.imgtec.com/powervr/insider/powervr-pvrtextool.asp)
# location of etc1tool (android-sdk-linux/tools)

#where the script is
#assume default layout: unprepared_assets and assets in the same folder
current=$(pwd)
whereAmI=$(cd "$(dirname "$0")" && pwd)
#if we executed a linked script; go to the real one
if [ -h $0 ]; then
    whereAmI+=/$(dirname $(readlink $0))
fi
outPath="$current/.."

#import cool stuff
source $whereAmI/cool_stuff.sh

export SAC_USAGE="$0 image_folder1 image_folder2 image..."
export SAC_OPTIONS="Note: you can use * for listing directories"
export SAC_EXAMPLE="${green}$0 unprepared_assets/logo"

if [ $# = 0 ]; then
    usage_and_quit
fi

############# STEP 0: verify env
check_package_in_PATH "texture_packer" "sac binary! (build/ directory)"
hasPVRTool=false
if hash PVRTexToolCL 2>/dev/null; then
    info "PVRTexToolCL found."
    hasPVRTool=true
    dpis="hdpi mdpi ldpi"
else
    info "Warning: PVRTexToolCL not found -> compressed format won't be created" $orange
    dpis="hdpi"
fi

current=0
for directory_path in $@; do
    dir=$(basename $directory_path)
    current=$(expr $current + 1)
    info "Treating atlas $dir at path $directory_path... ($current / $# - $(expr $current \* 100 / $# )%)"
    TEMP_FOLDER=$(mktemp)

    if [ ! -d "$directory_path" ]; then
        error_and_quit "Directory $directory_path does not exist!"
    fi
    if [ -z "$(find $directory_path -name '*.png')" ]; then
        error_and_quit "Directory $directory_path does not contain any .png file!"
    fi

    divide_by=1

    TMP_FILEDIR=$(mktemp)
    for quality in ${dpis}; do
        info "Generate $quality atlas"
        
        ############# STEP 1: preparation
        info "Step #1: prepare temp folder ($TEMP_FOLDER)"
        rm -rf $TEMP_FOLDER 2>/dev/null
        mkdir $TEMP_FOLDER

        rm -rf $TMP_FILEDIR 2>/dev/null
        mkdir $TMP_FILEDIR/$quality -p
        mkdir $TMP_FILEDIR/tmp -p

        mkdir $outPath/assets/$quality -p
        mkdir $outPath/assetspc/$quality -p

        find $outPath/assets/${quality}/ -name "${dir}*" -exec rm {} \;
        find $outPath/assets/${quality}/ -name "${dir}*" -exec rm {} \;
        
        ############# STEP 2: create an optimized copy of each image
        info "Step #2: create optimized image"
        for file in $(cd $directory_path && ls *.png); do
            width=$(identify -format "%w" $directory_path/$file)
            scale=$(($width/$divide_by))
            convert $directory_path/$file -scale $scale PNG32:$TMP_FILEDIR/$file
        done
        for file in $(cd $TMP_FILEDIR && ls *.png); do
            info "\tOptimizing $file..." $blue
            used_rect=$($whereAmI/texture_packer/tiniest_rectangle.py $TMP_FILEDIR/${file})

            if [ $? != 0 ]; then
                error_and_quit "Script encountered an error with file $file! Aborting"
            fi

            ww=$(echo $used_rect | cut -d, -f1)
            hh=$(echo $used_rect | cut -d, -f2)
            xx=$(echo $used_rect | cut -d, -f3)
            yy=$(echo $used_rect | cut -d, -f4)

            if (! convert -crop ${ww}x${hh}+${xx}+${yy} +repage $TMP_FILEDIR/$file PNG32:$TEMP_FOLDER/${file}); then
                error_and_quit "Error when converting $file!"
            fi

        done

        ############# STEP 3: fit all image in one texture
        # Remark: we feed texture_packer with optimized (cropped) images
        info "Step #3: compute atlas placement"
        optimized_image_list=$(echo $TEMP_FOLDER/*png)

        ############# STEP 4: place image in atlas
        # Remark: we first cd into original image folder
        info "Step #4: compose atlas using individual images coords"
        cd $TMP_FILEDIR
        texture_packer $optimized_image_list | $whereAmI/texture_packer/texture_packer.sh $dir
        cd - 1>/dev/null

        ############# STEP 5: create png version of the atlas
        info "Step #5: create png version"
        convert /tmp/$dir.png -alpha extract PNG24:$outPath/assetspc/$quality/${dir}_alpha.png
        convert /tmp/$dir.png -background white -alpha off -type TrueColor PNG24:$outPath/assetspc/$quality/$dir.png
        
        if  $hasPVRTool ; then
            info "Step #6: create ETC version of color texture"
            PVRTexToolCL -f ETC -yflip0 -i $outPath/assetspc/$quality/$dir.png -q 3 -pvrlegacy -o ${TMP_FILEDIR}/tmp/$dir-$quality.pkm
            # PVRTexToolCL ignore name extension
            split -d -b 1024K ${TMP_FILEDIR}/tmp/$dir-$quality.pvr ${TMP_FILEDIR}/$quality/$dir.pkm.

            info "Step #7: create ETC version of alpha texture"
            PVRTexToolCL -f ETC -yflip0 -i $outPath/assetspc/$quality/${dir}_alpha.png -q 3 -pvrlegacy -o ${TMP_FILEDIR}/tmp/${dir}_alpha-$quality.pkm
            # PVRTexToolCL ignore name extension
            split -d -b 1024K ${TMP_FILEDIR}/tmp/${dir}_alpha-$quality.pvr ${TMP_FILEDIR}/$quality/${dir}_alpha.pkm.
        fi
        
        cp -rv ${TMP_FILEDIR}/$quality $outPath/assets
        cp /tmp/$dir.atlas $outPath/assets/$quality/

        rm -r $TEMP_FOLDER/*
        divide_by=$(($divide_by * 2))
    done
done

rm -r $TEMP_FOLDER

info "Cya!"

