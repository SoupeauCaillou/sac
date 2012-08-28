#!/bin/sh

if [ -z "$1" ]; then
	echo "Need translations files ! Abording. Example : "
	echo "\tLANG:default:fr:it:de"
	echo "\tdiff_1:easy:facile:facillio:uberswuz"
	echo "\tetc…"
	echo "Auf wiedersehen !"
	exit
fi

if [ -z "$2" ]; then
	echo "Need res path. Example : code/c/heriswap/heriswap/res"
	echo "Auf wiedersehen !"
	exit
fi

echo "remove old values dir ? (y/n)"
read confirm
if [ "$confirm" = "n" ]; then
	echo "aborded"
	exit
fi

rm -r $2/values*

rm -r /tmp/aaaaaaaaaaaaa
mkdir /tmp/aaaaaaaaaaaaa

firstLine=`head -n 1 "$1"`
number=`echo "$firstLine" | grep -o "~" | wc -l`
for i in `seq $number`; do
	mkdir /tmp/aaaaaaaaaaaaa/values-$i
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<resources>" > /tmp/aaaaaaaaaaaaa/values-$i/strings.xml
done

#~for i in `tail -n +2 "$1"`; do
while read line
do
	if [ "$line" != "languages~en~fr~de~it" ]; then
		keyWord=`echo $line | cut -d"~" -f1`
		for j in `seq $number`; do
			jPlusUn=`expr $j + 1`
			echo "\t<string name=\"$keyWord\">`echo $line | cut -d"~" -f $jPlusUn`</string>" >> /tmp/aaaaaaaaaaaaa/values-$j/strings.xml
		done
	fi
done < $1



#rename dir by their lang name
for i in `seq $number`; do
	echo "</resources>" >> /tmp/aaaaaaaaaaaaa/values-$i/strings.xml
	iPlusUn=`expr $i + 1`
	lang=`echo $firstLine | cut -d"~" -f "$iPlusUn"`
	mv /tmp/aaaaaaaaaaaaa/values-$i $2/values-$lang
done

mv $2/values-en $2/values
rm -r /tmp/aaaaaaaaaaaaa
