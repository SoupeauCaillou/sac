#!/bin/sh

# path need :
# location of texture_packer (your_build_dir/sac/build/cmake)
# location of PVRTexToolCL (need to install it from http://www.imgtec.com/powervr/insider/powervr-pvrtextool.asp)
# location of etc1tool (android-sdk-linux/tools)

reset="[0m"
red="[1m[31m"
green="[1m[32m"
yellow="[1m[33m"

if [ $# != 1 ]; then
	echo "${red}Usage: $0 image_folder${reset}"
	exit 1
fi

############# STEP 0: verify env
if hash texture_packer 2>/dev/null; then
    echo "texture_packer found!"
else
    echo "${red}texture_packer must be in PATH$reset"
    exit 1
fi

hasPVRTool=false
if hash PVRTexToolCL 2>/dev/null; then
    echo "PVRTexToolCL found!"
    hasPVRTool=true
else
    echo "${yellow}Warning: PVRTexToolCL not found -> compressed format won't be created$reset"
fi

SCRIPT=$(readlink -f $0)
SAC_TOOLS_DIR=$(dirname $SCRIPT)
TEMP_FOLDER=/tmp/$1

############# STEP 1: preparation
echo "${green}Step #1: prepare temp folder ($TEMP_FOLDER)${reset}"
if [ -d "$TEMP_FOLDER" ]; then
    rm -r $TEMP_FOLDER
fi
mkdir $TEMP_FOLDER

############# STEP 2: create an optimized copy of each image
printf "${green}Step #2: create optimized image${reset}"
for file in `ls $1/*png`
do
    printf "${green}.${reset}"
    used_rect=`${SAC_TOOLS_DIR}/texture_packer/tiniest_rectangle.py ${file}`
    ww=`echo $used_rect | cut -d, -f1`
    hh=`echo $used_rect | cut -d, -f2`
    xx=`echo $used_rect | cut -d, -f3`
    yy=`echo $used_rect | cut -d, -f4`
    convert -crop ${ww}x${hh}+${xx}+${yy} +repage ${file} PNG32:$TEMP_FOLDER/`basename ${file}`
done
echo "${green}Done${reset}"

############# STEP 3: fit all image in one texture
# Remark: we feed texture_packer with optimized (cropped) images
echo "${green}Step #3: compute atlas placement${reset}"
optimized_image_list=$(ls $TEMP_FOLDER/*png)

############# STEP 4: place image in atlas
# Remark: we first cd into original image folder
echo "${green}Step #4: compose atlas using individual images coords${reset}"
oldpwd=$PWD
cd $1
texture_packer $optimized_image_list | ${SAC_TOOLS_DIR}/texture_packer/texture_packer.sh $1
cd $oldpwd

############# STEP 5: create png version of the atlas
echo "${green}Step #5: create png version${reset}"
convert /tmp/$1.png -alpha extract -depth 8 ../assets/$1_alpha.png
# mais pourquoi j'ai fait ca ?convert ../assets/$1_alpha.png -background white -flatten +matte -depth 8 ../assets/$1_alpha.png
convert /tmp/$1.png -background white -alpha off -type TrueColor PNG24:../assetspc/$1.png

if  $hasPVRTool ; then
    echo "${green}Step #6: create PVR version${reset}"
    PVRTexToolCL -f OGLPVRTC4 -yflip0 -i ../assetspc/$1.png -p -pvrlegacy -m -o /tmp/$1.pvr
    split -d -b 1024K /tmp/$1.pvr $1.pvr.
    mv $1.pvr.0* ../assets/

    echo "${green}Step #7: create ETC version${reset}"
    PVRTexToolCL -f ETC -yflip0 -i ../assetspc/$1.png -q 3 -m -pvrlegacy -o /tmp/$1.pkm

    # PVRTexToolCL ignore name extension
    split -d -b 1024K /tmp/$1.pvr $1.pkm.
    mv $1.pkm.0* ../assets/
fi

cp /tmp/$1.atlas ../assets/
