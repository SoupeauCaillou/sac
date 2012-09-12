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

if [ -z "$3" ]; then
	echo "Need separator char. Example (~) : \tdiff_1~easy~facile~facillio~uberswuz"
	echo "Auf wiedersehen !"
	exit
fi
separateur=$3
echo "The separator is : $separateur (missed the \ character ?)"

echo "remove old values dir ? (y/n)"
read confirm
if [ "$confirm" = "n" ]; then
	echo "aborded"
	exit
fi

echo "rm -r $2/values*"
rm -r $2/values* 2>/dev/null

rm -r /tmp/xmlFromTable 2>/dev/null
mkdir /tmp/xmlFromTable

firstLine=`head -n 1 "$1"`
number=`echo "$firstLine" | grep -o "$separateur" | wc -l`
for i in `seq $number`; do
	echo "mkdir /tmp/xmlFromTable/values-$i"
	mkdir /tmp/xmlFromTable/values-$i
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<resources>" > /tmp/xmlFromTable/values-$i/strings.xml
done

while read line
do
	if [ "$line" != "languages$separateuren$separateurfr$separateurde$separateurit$separateurnl$separateures" ]; then
		keyWord=`echo $line | cut -d$separateur -f1`
		for j in `seq $number`; do
			jPlusUn=`expr $j + 1`
			translation=`echo $line | cut -d$separateur -f $jPlusUn`
			
			#fill with english transl if there isn't one in this language
			if [ -z "$translation" ]; then
				translation=`echo $line | cut -d$separateur -f2`				
			fi
			
			echo "\t<string name=\"$keyWord\">\"$translation\"</string>" >> /tmp/xmlFromTable/values-$j/strings.xml
		done
	fi
done < $1



#rename dir by their lang name
for i in `seq $number`; do
	echo "</resources>" >> /tmp/xmlFromTable/values-$i/strings.xml
	iPlusUn=`expr $i + 1`
	lang=`echo $firstLine | cut -d$separateur -f "$iPlusUn"`
	echo "mv /tmp/xmlFromTable/values-$i $2/values-$lang"
	mv /tmp/xmlFromTable/values-$i $2/values-$lang
done

mv $2/values-en $2/values
rm -r /tmp/xmlFromTable
