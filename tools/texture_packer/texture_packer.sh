#!/bin/bash

reset="[0m"
red="[1m[31m"
blue="[1m[34m"

if [ $# -ne 1 ]
then
	echo "Usage: $0 atlas_to_create"
	exit 1
fi

SCRIPT=$(readlink -f $0)
SAC_TOOLS_TEXTURE_DIR=$(dirname $SCRIPT)

output=/tmp/${1}.png
desc=/tmp/${1}.atlas
tmp_image=/tmp/tmp___.png

if [ -f "${desc}" ]; then
    rm ${desc}
fi

COUNTER=0

# Take the output of texture_packer as input and then use image magick to compose the atlas
while read data; do
	if echo $data | grep -q Atlas
	then
		# create image
		w=`echo $data | cut -d: -f2 | cut -d, -f1`
		h=`echo $data | cut -d, -f2`

		convert -size ${w}x${h} xc:transparent ${output}
		echo "\tatlas_size=$w,$h" > ${desc}
	else
        #read input from pipe
        #image is an absolute path
		image=`echo $data | cut -d, -f1`
		x=`echo $data | cut -d, -f2`
		y=`echo $data | cut -d, -f3`
		w=`echo $data | cut -d, -f4`
		h=`echo $data | cut -d, -f5`
		rot=`echo $data | cut -d, -f6`

        # extract image name (without extension)
        base=`basename ${image} .png`
        # find out original (unoptimized) image size
        real_size=`identify ${base}.png | cut -d\  -f3 | sed 's/x/,/'`
        # compute optimized area in image
        used_rect=`${SAC_TOOLS_TEXTURE_DIR}/tiniest_rectangle.py ${base}.png`
        used_rect_w=`echo ${used_rect} | cut -d, -f1`
        used_rect_h=`echo ${used_rect} | cut -d, -f2`
        used_rect_x=`echo ${used_rect} | cut -d, -f3`
        used_rect_y=`echo ${used_rect} | cut -d, -f4`

		if [ ${rot} -ne "0" ]
		then
			convert -rotate 90 ${image} PNG32:${tmp_image}
		else
			cp ${image} ${tmp_image}
		fi

		echo "   ${blue}Adding ${image} at ${w}x${h}+${x}+${y} (rotation:${rot})${reset}"
		# copy a first version, 2 pixel taller/larger (used as a blend-compatible border)
		convert -geometry `expr ${w} + 2;`x`expr ${h} + 2;`+`expr ${x} - 1;`+`expr ${y} - 1;` -compose Copy -composite $output $tmp_image $output
		# copy the real image
		convert -geometry ${w}x${h}+${x}+${y} -compose Copy -composite $output $tmp_image $output


		#largest_rectangle script
		opaque=`$SAC_TOOLS_TEXTURE_DIR/largest_rectangle.py ${image}`

        echo "[image$COUNTER]" >> ${desc}
        echo "name=${base}" >> ${desc}
        echo "original_size=${real_size}" >> ${desc}
        echo "position_in_atlas=${x},${y}" >> ${desc}
        echo "size_in_atlas=${w},${h}" >> ${desc}
        echo "crop_offset=${used_rect_x},${used_rect_y}" >> ${desc}
        echo "rotated=${rot}" >> ${desc}
        if [ -n "$opaque" ]; then
            echo "opaque_rect=${opaque}" >> ${desc}
        fi
        let COUNTER=COUNTER+1
	fi
done
