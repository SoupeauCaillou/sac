#!/bin/bash

#where the script is
#assume default layout: unprepared_assets and assets in the same folder
current=$(pwd)
whereAmI=$(cd "$(dirname "$0")" && pwd)
#if we executed a linked script; go to the real one
if [ -h $0 ]; then
    whereAmI+=/$(dirname $(readlink $0))
fi
outPath="$whereAmI/../.."

#import cool stuff
source $whereAmI/cool_stuff.sh

export SAC_USAGE="$0 image_folder1 image_folder2 image..."
export SAC_OPTIONS="\
--q|--quality (hdpi / mdpi / ldpi): atlas to build.
\tNote: you can use * for listing directories."
export SAC_EXAMPLE="${green}$0 unprepared_assets/logo --a \"hdpi mdpi\""

if [ $# = 0 ]; then
    usage_and_quit
fi

############# STEP 0: verify env and args
check_package_in_PATH "texture_packer" "sac binary! (build/ directory)"

if ! $(python -c "import PIL" &> /dev/null); then
    check_package "python-pil"
fi

hasetc1tool=false
if check_package_in_PATH etc1tool '$ANDROID_HOME/tools/' DONT_EXIT; then
    dpis="hdpi mdpi ldpi"
    hasetc1tool=true
else
    info "Warning: etc1tool not found -> compressed format won't be created" $orange
    dpis="hdpi"
fi

hasNVTool=false
if check_package nvcompress 'https://code.google.com/p/nvidia-texture-tools/ then $BUILD/src/nvtt/tools/' DONT_EXIT; then
    info "nvcompress found."
    hasNVTool=true
else
    info "Warning: nvcompress missing" $orange
fi

while [ "$1" != "" ]; do
    case $1 in
        "--q" | "--quality")
            shift
            dpis="$1"
            ;;
        *)
            directories+=($1)
    esac
    shift
done

info "Setup #0 done: will build '${dpis}' atlas."

############# STEP 1: process
current=0
tcount=${#directories[@]}
for directory_path in "${directories[@]}"; do
    dir=$(basename $directory_path)
    current=$(expr $current + 1)
    info "Treating atlas $dir at path $directory_path... ($current / $tcount - $(expr $current \* 100 / $tcount )%)"
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

        ############# SUBSTEP 1: preparation
        info "Substep #1: prepare temp folder ($TEMP_FOLDER)"
        rm -rf $TEMP_FOLDER 2>/dev/null
        mkdir $TEMP_FOLDER

        rm -rf $TMP_FILEDIR 2>/dev/null
        mkdir $TMP_FILEDIR/$quality -p
        mkdir $TMP_FILEDIR/tmp -p

        mkdir $outPath/assets/$quality -p

        find $outPath/assets/${quality}/ -name "${dir}*" -exec rm {} \;
        find $outPath/assets/${quality}/ -name "${dir}*" -exec rm {} \;

        ############# SUBSTEP 2: create an optimized copy of each image
        info "Substep #2: create optimized image"
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

        ############# SUBSTEP 3: fit all image in one texture
        # Remark: we feed texture_packer with optimized (cropped) images
        info "Substep #3: compute atlas placement"
        optimized_image_list=$(echo $TEMP_FOLDER/*png)

        ############# SUBSTEP 4: place image in atlas
        # Remark: we first cd into original image folder
        info "Substep #4: compose atlas using individual images coords"
        cd $TMP_FILEDIR
        texture_packer $optimized_image_list | $whereAmI/texture_packer/texture_packer.sh $dir
        cd - 1>/dev/null

        ############# SUBSTEP 5: create png version of the atlas
        info "Substep #5: create png version"
        # Pre-multiplied alpha version
        convert /tmp/$dir.png \( +clone -alpha Extract \) -channel RGB -compose Multiply -composite /tmp/$dir.png
        # Alpha only image
        convert /tmp/$dir.png -alpha extract PNG24:/tmp/${dir}_alpha.png
        # Color only image
        convert /tmp/$dir.png -background white -alpha off -type TrueColor PNG24:/tmp/$dir.png


        if $hasetc1tool ; then
            info "Substep #6a: create ETC version of color texture"
            etc1tool --encode /tmp/$dir.png -o ${TMP_FILEDIR}/tmp/$dir-$quality.pkm
            # PVRTexToolCL ignore name extension
            split -d -b 1024K ${TMP_FILEDIR}/tmp/$dir-$quality.pkm ${TMP_FILEDIR}/$quality/$dir.pkm.

            info "Substep #6b: create ETC version of alpha texture"
            etc1tool --encode /tmp/${dir}_alpha.png -o ${TMP_FILEDIR}/tmp/${dir}_alpha-$quality.pkm
            # PVRTexToolCL ignore name extension
            split -d -b 1024K ${TMP_FILEDIR}/tmp/${dir}_alpha-$quality.pkm ${TMP_FILEDIR}/$quality/${dir}_alpha.pkm.
        fi

        if $hasNVTool ; then
            info "Substep #7a: create DDS version of color texture"
            nvcompress -bc1 -color -nomips -silent /tmp/${dir}.png ${TMP_FILEDIR}/tmp/$dir-$quality.dds
            # PVRTexToolCL ignore name extension
            split -d -b 1024K ${TMP_FILEDIR}/tmp/$dir-$quality.dds ${TMP_FILEDIR}/$quality/$dir.dds.

            info "Substep #7b: create DDS version of alpha texture"
            nvcompress -bc1 -color -nomips -silent /tmp/${dir}_alpha.png ${TMP_FILEDIR}/tmp/${dir}_alpha-$quality.dds
            # PVRTexToolCL ignore name extension
            split -d -b 1024K ${TMP_FILEDIR}/tmp/${dir}_alpha-$quality.dds ${TMP_FILEDIR}/$quality/${dir}_alpha.dds.
        fi

        cp -rv ${TMP_FILEDIR}/$quality $outPath/assets
        cp /tmp/$dir.atlas $outPath/assets/$quality/

        rm -r $TEMP_FOLDER/*
        divide_by=$(($divide_by * 2))
    done
done

rm -r $TEMP_FOLDER

info "Cya!"

