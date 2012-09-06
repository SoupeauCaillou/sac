#!/bin/sh

separateur="~"

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

echo "rm -r $2/values*"
rm -r $2/values*

rm -r /tmp/aaaaaaaaaaaaa 2>/dev/null
mkdir /tmp/aaaaaaaaaaaaa

firstLine=`head -n 1 "$1"`
number=`echo "$firstLine" | grep -o "$separateur" | wc -l`
for i in `seq $number`; do
	echo "mkdir /tmp/aaaaaaaaaaaaa/values-$i"
	mkdir /tmp/aaaaaaaaaaaaa/values-$i
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<resources>" > /tmp/aaaaaaaaaaaaa/values-$i/strings.xml
done

while read line
do
	if [ "$line" != "languages$separateuren$separateurfr$separateurde$separateurit$separateurnl$separateures" ]; then
		keyWord=`echo $line | cut -d"$separateur" -f1`
		for j in `seq $number`; do
			jPlusUn=`expr $j + 1`
			echo "\t<string name=\"$keyWord\">`echo $line | cut -d\"$separateur\" -f $jPlusUn`</string>" >> /tmp/aaaaaaaaaaaaa/values-$j/strings.xml
		done
	fi
done < $1



#rename dir by their lang name
for i in `seq $number`; do
	echo "</resources>" >> /tmp/aaaaaaaaaaaaa/values-$i/strings.xml
	iPlusUn=`expr $i + 1`
	lang=`echo $firstLine | cut -d"$separateur" -f "$iPlusUn"`
	mv /tmp/aaaaaaaaaaaaa/values-$i $2/values-$lang
done

mv $2/values-en $2/values
rm -r /tmp/aaaaaaaaaaaaa
