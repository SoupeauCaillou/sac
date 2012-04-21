#!/bin/sh

if [ $# -ne 1 ]
then
	echo "Usage: font_desc_generator.sh typo_name"
	exit 1
fi

echo "#FONT: ${1}"
echo "#char,width,height"
for i in `ls *_${1}.png`;
do
	size=`identify $i | cut -d\  -f 3`
	c=`echo $i | cut -d_ -f1`
	width=`echo $size | cut -dx -f1`
	height=`echo $size | cut -dx -f2`
	echo "$c,$width,$height"
done
