#!/bin/bash

# path need :
# location of texture_packer (your_build_dir/sac/build/cmake)
# location of PVRTexToolCL (need to install it from http://www.imgtec.com/powervr/insider/powervr-pvrtextool.asp)
# location of etc1tool (android-sdk-linux/tools)

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
else
    info "Warning: PVRTexToolCL not found -> compressed format won't be created" $orange
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

    ############# STEP 1: preparation
    info "Step #1: prepare temp folder ($TEMP_FOLDER)"
    rm -rf $TEMP_FOLDER 2>/dev/null
    mkdir $TEMP_FOLDER

    rm $rootPath/assets/${dir}_alpha.png $rootPath/assets/${dir}.atlas $rootPath/assets/${dir}.pvr.00 \
    $rootPath/assets/${dir}.pkm.00 $rootPath/assetspc/${dir}.png 2>/dev/null
    ############# STEP 2: create an optimized copy of each image
    info "Step #2: create optimized image"
    for file in $(cd $directory_path && ls *.png); do
        info "\tOptimizing $file..." $blue
        used_rect=$($whereAmI/texture_packer/tiniest_rectangle.py $directory_path/${file})

        if [ $? != 0 ]; then
            error_and_quit "Script encountered an error with file $file! Aborting"
        fi

        ww=$(echo $used_rect | cut -d, -f1)
        hh=$(echo $used_rect | cut -d, -f2)
        xx=$(echo $used_rect | cut -d, -f3)
        yy=$(echo $used_rect | cut -d, -f4)

        if (! convert -crop ${ww}x${hh}+${xx}+${yy} +repage $directory_path/$file PNG32:$TEMP_FOLDER/${file}); then
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
    cd $directory_path
    texture_packer $optimized_image_list | $whereAmI/texture_packer/texture_packer.sh $dir
    cd - 1>/dev/null

    ############# STEP 5: create png version of the atlas
    info "Step #5: create png version"
    convert /tmp/$dir.png -alpha extract PNG32:$rootPath/assets/${dir}_alpha.png
    convert /tmp/$dir.png -background white -alpha off -type TrueColor PNG24:$rootPath/assetspc/$dir.png
    
    width=$(file $rootPath/assetspc/$dir.png | cut -d ',' -f2 | cut -d ' ' -f2)

    divide_by=1
    tmp_png=/tmp/$dir-copy.png
    if  $hasPVRTool ; then
        info "Step #6: create PVR and ETC version"
        for i in  "hdpi" "mdpi" "ldpi"; do
            convert $rootPath/assetspc/$dir.png -scale $(($width/$divide_by)) $tmp_png
            info "Create $i version"
            mkdir /tmp/$i -p
            PVRTexToolCL -f PVRTC2_4 -flip y,flag -i $tmp_png -p -m -o /tmp/$dir-$i.pvr -shh
            split -d -b 1024K /tmp/$dir-$i.pvr /tmp/$i/$dir.pvr.
            cp -r /tmp/$i $rootPath/assets/

            PVRTexToolCL -f ETC1 -flip y,flag -i $tmp_png -q pvrtcbest -m -o /tmp/$dir-$i-pkm.pvr -shh

            # PVRTexToolCL ignore name extension
            split -d -b 1024K /tmp/$dir-$i-pkm.pvr /tmp/$i/$dir.pkm.
            cp -r /tmp/$i/ $rootPath/assets/

            divide_by=$(($divide_by * 2))
        done 
    fi

    cp /tmp/$dir.atlas $rootPath/assets/

    rm -r $TEMP_FOLDER/*
done

rm -r $TEMP_FOLDER

info "Cya!"

