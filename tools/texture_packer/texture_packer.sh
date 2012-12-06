#!/bin/sh

if [ $# -ne 1 ]
then
	echo "Usage: texture_packer atlas_to_create"
	exit 1
fi

output=/tmp/${1}.png
#output_alpha=/tmp/${1}_alpha.png
desc=/tmp/${1}.desc
tmp_image=/tmp/tmp___.png

rm ${desc}

# Take the output of texture_packer as input and then use image magick to compose the atlas
while read data; do
	if echo $data | grep -q Atlas
	then
		# create image
		w=`echo $data | cut -d: -f2 | cut -d, -f1`
		h=`echo $data | cut -d, -f2`
		echo "Atlas size: $w x $h"
		convert -size ${w}x${h} xc:transparent ${output}
		#convert -size ${w}x${h} xc:transparent ${output_alpha}
		echo "$w,$h" > ${desc}
	else
		image=`echo $data | cut -d, -f1`
		x=`echo $data | cut -d, -f2`
		y=`echo $data | cut -d, -f3`
		w=`echo $data | cut -d, -f4`
		h=`echo $data | cut -d, -f5`
		rot=`echo $data | cut -d, -f6`

		if [ ${rot} -ne "0" ]
		then
			convert -rotate 90 ${image} ${tmp_image}
		else
			cp ${image} ${tmp_image}
		fi

		echo "Adding ${image} at ${w}x${h}+${x}+${y} (rotation:${rot})"
		# copy a first version, 2 pixel taller/larger (used as a blend-compatible border)
		convert -geometry `expr ${w} + 2;`x`expr ${h} + 2;`+`expr ${x} - 1;`+`expr ${y} - 1;` -composite $output $tmp_image $output
		# copy the real image
		convert -geometry ${w}x${h}+${x}+${y} -compose Dst -composite $output $tmp_image $output
		#convert -geometry ${w}x${h}+${x}+${y} -composite $output_alpha $tmp_image $output_alpha
		image=`basename ${image} .png`
		
		#largest_rectangle script 
		opaque=`../../sac/tools/texture_packer/largest_rectangle.py ${image}.png`
        used_rect=`../../sac/tools/texture_packer/tiniest_rectangle.py ${image}.png`

		if [ -n "$opaque" ]; then
			echo "${image},${x},${y},${w},${h},${rot},opaque,${opaque}" >> ${desc}
		else
			echo "${image},${x},${y},${w},${h},${rot},sub,${used_rect}" >> ${desc}
		fi
	fi
done
