#!/bin/sh

### FUNCTIONS
usage() {
	echo "Usage:   sh xmlLanguageFromTable.sh"
	echo "Bybye every body!"
	exit
}

error_and_quit() {
	echo "######## $1 ########"
	usage
}

######### Start : check arguments
if [ $# != 0 ]; then
	error_and_quit "Wrong parameters count"
fi

######### FIRST : download the google doc file ######
	#LINKS ARE HARD LINKED
	Hdrive="docs.google.com/spreadsheet/ccc?key=0AiBDfxibD5bHdGVHRzFLSjJLRVZ1bmNWY3R0Z2VieUE&output=txt"
	RRdrive="docs.google.com/spreadsheet/ccc?key=0AmvXloUjXatzdC1TQXJtcGxCOGo0cG5nVFBMMTlRQXc&output=txt"

	csvFILE=/tmp/tmp$PPID.txt

	GAME_IS=`cd "$(dirname "$0")" && cd ../.. && pwd`
	if [ `echo "$GAME_IS" | grep -i "heriswap"` ]; then
		#heriswap
		wget "$Hdrive" -O $csvFILE
	elif [ `echo "$GAME_IS" | grep -i "runner"` ]; then
		#RR
		wget "$RRdrive" -O $csvFILE
	else
		error_and_quit "This is NOR heriswap and recursiveRunner ($PWD doesn't contain both of us). abort"
	fi

	#by default, google save the docs with tabulations so we need to change \t to ~
	#(even if it could work with tabulations)
	sed -i 's/\t/~/g' $csvFILE
	#and set a \n at the end of the file (google doesn't)
	echo '' >> $csvFILE

######## SECOND : treat the csv file…
	#- echo "remove old values*/strings.xml files ? (y/n)"
	#- read confirm
	#- if [ "$confirm" != "y" ]; then
		#- echo "aborded"
		#- exit
	#- fi

	echo "rm -r $GAME_IS/res/values*/strings.xml"
	rm -r $GAME_IS/res/values*/strings.xml 2>/dev/null

	rm -r /tmp/xmlFromTable 2>/dev/null
	mkdir /tmp/xmlFromTable

	separateur=\~
	firstLine=`head -n 1 "$csvFILE"`
	number=`echo "$firstLine" | grep -o "$separateur" | wc -l`
	for i in `seq $number`; do
		echo "mkdir /tmp/xmlFromTable/values-$i"
		mkdir /tmp/xmlFromTable/values-$i
		echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<resources>" > /tmp/xmlFromTable/values-$i/strings.xml
	done

	while read -r line; do
		#don't keep the "language" line
		if [ -z "`echo $line | grep languages | grep en`" ]; then
			keyWord=`printf '%s' "$line" | cut -d$separateur -f1`
			for j in `seq $number`; do
				jPlusUn=`expr $j + 1`
				translation=`printf '%s' "$line" | cut -d$separateur -f $jPlusUn`

				#fill with english transl if there isn't one in this language
				if [ -z "$translation" ]; then
					translation=`printf '%s\n' "$line" | cut -d$separateur -f2`
				fi
				printf '\t<string name="%s">"%s"</string>\n' "$keyWord" "$translation" >> /tmp/xmlFromTable/values-$j/strings.xml
			done
		fi
	done < $csvFILE

	#rename dir by their lang name
	for i in `seq $number`; do
		echo "</resources>" >> /tmp/xmlFromTable/values-$i/strings.xml
		iPlusUn=`expr $i + 1`
		lang=`echo $firstLine | cut -d$separateur -f "$iPlusUn"`
		echo "mv /tmp/xmlFromTable/values-$i $GAME_IS/res/values-$lang"

		#2 use case here: the res already exist (and then we juste move
		#the file), or not (and we need to move the entire folder)
		if [ -d $GAME_IS/res/values-$lang ]; then
			mv /tmp/xmlFromTable/values-$i/strings.xml $GAME_IS/res/values-$lang/
		else
			mv /tmp/xmlFromTable/values-$i $GAME_IS/res/values-$lang
		fi
	done

	if [ -d $GAME_IS/res/values ]; then
		mv $GAME_IS/res/values-en/* $GAME_IS/res/values
		rm -r $GAME_IS/res/values-en
	else
		mv $GAME_IS/res/values-en $GAME_IS/res/values
	fi

	#then clean tmp files..
	rm -r /tmp/xmlFromTable

	rm $csvFILE
