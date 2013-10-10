#!/bin/bash

#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
#if we executed a linked script; go to the real one
if [ -h $0 ]; then
    whereAmI+=/$(dirname $(readlink $0))
fi
rootPath=$whereAmI"/../../.."
gameName=$(cat $rootPath/CMakeLists.txt | grep 'project(' | cut -d '(' -f2 | tr -d ')')

#import cool stuff
source $whereAmI/../coolStuff.sh

export SAC_USAGE="$0 atlas_to_create"
export SAC_OPTIONS=""
export SAC_EXAMPLE="${green}$0 trucbidule"

if [ $# != 1 ]; then
    usage_and_quit
fi

output=/tmp/${1}.png
desc=/tmp/${1}.atlas
tmp_image=$(mktemp).png

if [ -f "${desc}" ]; then
    rm ${desc}
fi

COUNTER=0

# Take the output of texture_packer as input and then use image magick to compose the atlas
while read data; do
	if grep -q Atlas <<< $data ; then
		# create image
		w=$(echo $data | cut -d: -f2 | cut -d, -f1)
		h=$(echo $data | cut -d, -f2)

		convert -size ${w}x${h} xc:transparent PNG32:${output}
		echo "atlas_size=$w,$h" > ${desc}
	else
        #read input from pipe
        #image is an absolute path
		image=$(echo $data | cut -d, -f1)
		x=$(echo $data | cut -d, -f2)
		y=$(echo $data | cut -d, -f3)
		w=$(echo $data | cut -d, -f4)
		h=$(echo $data | cut -d, -f5)
		rot=$(echo $data | cut -d, -f6)

        # extract image name (without extension)
        base=$(basename ${image} .png)
        # find out original (unoptimized) image size
        real_size=$(identify ${base}.png | cut -d\  -f3 | sed 's/x/,/')
        # compute optimized area in image
        used_rect=$($whereAmI/tiniest_rectangle.py ${base}.png)

        if [ $? != 0 ]; then
            error_and_quit "Script encountered an error with file $base! Aborting"
        fi

        used_rect_w=$(echo ${used_rect} | cut -d, -f1)
        used_rect_h=$(echo ${used_rect} | cut -d, -f2)
        used_rect_x=$(echo ${used_rect} | cut -d, -f3)
        used_rect_y=$(echo ${used_rect} | cut -d, -f4)

		if [ "$rot" != "0" ]; then
			convert -rotate 90 ${image} PNG32:${tmp_image}
		else
			cp ${image} ${tmp_image}
		fi

		info "   Adding ${image} at ${w}x${h}+${x}+${y} (rotation:${rot})" $blue
		# copy a first version, 2 pixel taller/larger (used as a blend-compatible border)
		convert -geometry `expr ${w} + 2;`x`expr ${h} + 2;`+`expr ${x} - 1;`+`expr ${y} - 1;`! -compose Copy -composite $output $tmp_image PNG32:$output
		# copy the real image
		convert -geometry +${x}+${y} -compose Copy -composite $output $tmp_image PNG32:$output


		#largest_rectangle script
		opaque=$($whereAmI/largest_rectangle.py ${image})

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

cp $output /tmp/b.png
