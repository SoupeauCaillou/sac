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

export SAC_USAGE="$0 image_folder"
export SAC_OPTIONS=""
export SAC_EXAMPLE="${green}$0 unprepared_assets/logo"

if [ $# != 1 ]; then
    usage_and_quit
fi

############# STEP 0: verify env
check_package_in_PATH "texture_packer" "sac binary! (build/ directory)"
hasPVRTool=false
if hash PVRTexToolCL 2>/dev/null; then
    info "PVRTexToolCL found!"
    hasPVRTool=true
else
    info "Warning: PVRTexToolCL not found -> compressed format won't be created" $orange
fi

TEMP_FOLDER=$(mktemp)

############# STEP 1: preparation
info "Step #1: prepare temp folder ($TEMP_FOLDER)"
rm -rf $TEMP_FOLDER 2>/dev/null
mkdir $TEMP_FOLDER

rm $rootPath/assets/$1_alpha.png $rootPath/assets/$1.atlas $rootPath/assets/$1.pvr.00 \
$rootPath/assets/$1.pkm.00 $rootPath/assetspc/$1.png 2>/dev/null
############# STEP 2: create an optimized copy of each image
info "Step #2: create optimized image"
for file in $(echo $1/*png); do
    printf "." #loading...

    used_rect=$($whereAmI/texture_packer/tiniest_rectangle.py ${file})

    if [ $? != 0 ]; then
        error_and_quit "Script encountered an error with file $file! Aborting"
    fi

    ww=$(echo $used_rect | cut -d, -f1)
    hh=$(echo $used_rect | cut -d, -f2)
    xx=$(echo $used_rect | cut -d, -f3)
    yy=$(echo $used_rect | cut -d, -f4)
    convert -crop ${ww}x${hh}+${xx}+${yy} +repage ${file} PNG32:$TEMP_FOLDER/$(basename ${file})
done
info "Done"

############# STEP 3: fit all image in one texture
# Remark: we feed texture_packer with optimized (cropped) images
info "Step #3: compute atlas placement"
optimized_image_list=$(echo $TEMP_FOLDER/*png)

############# STEP 4: place image in atlas
# Remark: we first cd into original image folder
info "Step #4: compose atlas using individual images coords"
cd $1
texture_packer $optimized_image_list | $whereAmI/texture_packer/texture_packer.sh $1
cd - 1>/dev/null

############# STEP 5: create png version of the atlas
info "Step #5: create png version"
convert /tmp/$1.png -alpha extract PNG32:$rootPath/assets/$1_alpha.png

#convert /tmp/$1.png \( +clone -alpha Extract \) -channel RGB -compose Multiply -composite /tmp/$1.png

convert /tmp/$1.png -background white -alpha off -type TrueColor PNG24:$rootPath/assetspc/$1.png

if  $hasPVRTool ; then
    info "Step #6: create PVR version"
    PVRTexToolCL -f OGLPVRTC4 -yflip0 -i $rootPath/assetspc/$1.png -p -pvrlegacy -m -o /tmp/$1.pvr
    split -d -b 1024K /tmp/$1.pvr $1.pvr.
    mv $1.pvr.0* $rootPath/assets/

    info "Step #7: create ETC version"
    PVRTexToolCL -f ETC -yflip0 -i $rootPath/assetspc/$1.png -q 3 -m -pvrlegacy -o /tmp/$1.pkm

    # PVRTexToolCL ignore name extension
    split -d -b 1024K /tmp/$1.pvr $1.pkm.
    mv $1.pkm.0* $rootPath/assets/
fi

cp /tmp/$1.atlas $rootPath/assets/

rm -r $TEMP_FOLDER
