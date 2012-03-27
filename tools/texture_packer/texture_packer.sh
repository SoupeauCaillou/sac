#!/bin/sh

output=/tmp/atlas.png
tmp_image=/tmp/tmp___.png

# Take the output of texture packer as input and then use image magick to compose the atlas
while read data; do
	if echo $data | grep -q Atlas
	then
		# create image
		w=`echo $data | cut -d: -f2 | cut -d, -f1`
		h=`echo $data | cut -d, -f2`
		echo "Atlas size: $w x $h"
		convert -size ${w}x${h} xc:transparent ${output}
	else
		image=`echo $data | cut -d, -f1`
		x=`echo $data | cut -d, -f2`
		y=`echo $data | cut -d, -f3`
		w=`echo $data | cut -d, -f4`
		h=`echo $data | cut -d, -f5`
		rot=`echo $data | cut -d, -f6`
		
		if [ ${rot} ]
		then
			convert -rotate 90 ${image} ${tmp_image}
		else
			cp ${image} ${tmp_image}
		fi

		echo "Adding ${image} at ${w}x${h}+${x}+${y} (rotation:${rot})"
		convert -geometry ${w}x${h}+${x}+${y} -composite $output $tmp_image $output
	fi
done
