#!/bin/bash

######### Cool things #########
	#colors
	reset="[0m"
	red="[1m[31m"
	green="[1m[32m"

	info() {
		if [ $# = 1 ]; then
			echo "${green}$1${reset}"
		else
			echo "$2$1${reset}"
		fi
	}

	#get location of the script
	whereAmI=`cd "$(dirname "$0")" && pwd`


######### FUNCTIONS #########
	usage() {
		echo "Usage:   sh xmlLanguageFromTable.sh"
		echo "Bybye every body!"
		exit
	}

	error_and_quit() {
		info "######## $1 ########" $red
		usage
	}

######### 0 : Check arguments. #########
	if [ $# != 0 ]; then
		error_and_quit "Wrong parameters count"
	fi


separateur=\~

######### 1 : Download the google doc file. #########
	#LINKS ARE HARD LINKED
	Hdrive="docs.google.com/spreadsheet/ccc?key=0AiBDfxibD5bHdGVHRzFLSjJLRVZ1bmNWY3R0Z2VieUE&output=txt"
	RRdrive="docs.google.com/spreadsheet/ccc?key=0AmvXloUjXatzdC1TQXJtcGxCOGo0cG5nVFBMMTlRQXc&output=txt"

	csvFILE=/tmp/tmp$PPID.txt

	GAME_IS=`cd "$(dirname "$0")" && cd ../.. && pwd`
	if [ `echo "$GAME_IS" | grep -i "heriswap"` ]; then
		#heriswap
		info "wget HeriswapGoogleDoc"
		wget "$Hdrive" -O $csvFILE
	elif [ `echo "$GAME_IS" | grep -i "runner"` ]; then
		#RR
		info "wget RecursiveRunnerGoogleDoc"
		wget "$RRdrive" -O $csvFILE
	else
		error_and_quit "This is NOR heriswap and recursiveRunner ($PWD doesn't contain both of us). abort"
	fi

	#first delete empty lines
	count=`head -n 1 $csvFILE | grep -o "$separateur" | wc -l`
	pattern=""
	for i in `seq $count`; do
		pattern+='\t'
	done

	#we delete every lines beginning by the pattern
	sed -i '/^$pattern/d' $csvFILE

	#by default, google save the docs with tabulations so we need to change \t to ~
	#(even if it could work with tabulations)
	sed -i 's/\t/~/g' $csvFILE
	#and set a \n at the end of the file (google doesn't)
	echo '' >> $csvFILE

	cp $csvFILE ~/test.txt

######### 2 : Clean and setup the lab… #########

	#remove old strings from res
	echo "remove old values*/strings.xml files ? (y/n)"
	read confirm
	if [ "$confirm" != "y" ]; then
		echo "aborded"
		exit
	fi
	info "rm -r $GAME_IS/res/values*/strings.xml"
	rm -r $GAME_IS/res/values*/strings.xml 2>/dev/null


	#create tmp directories...
	rm -r /tmp/xmlFromTable 2>/dev/null
	mkdir /tmp/xmlFromTable


	#first line contains the language like : languages~en~fr~de~it~nl~es
	#it will be useful at the end to rename the directories correctly
	firstLine=`head -n 1 "$csvFILE"`

	#but at start we call directories values-1, values-2, values-3 ...
	langCount=`echo "$firstLine" | grep -o "$separateur" | wc -l`

	for i in `seq $langCount`; do
		info "mkdir /tmp/xmlFromTable/values-$i"
		mkdir /tmp/xmlFromTable/values-$i
		echo -e "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<resources>" > /tmp/xmlFromTable/values-$i/strings.xml
	done


######### 3 : Treat the csv file… #########

	#set an array of langCount size
	for j in `seq $langCount`; do
		translation[`expr $j + 1`]=""
	done

	#keyWord is the ID of the line; we save it because we can have a single ID
	#splitted in multipart.
	keyWord=""

	#read the file line per line
	while read -r line; do

		#don't keep the "language" line (first of the file, only used for the script)
		if [ ! -z "`echo $line | grep languages | grep en`" ]; then
			continue
		fi

		#the keyWord of the current line
		courKeyWord=`printf '%s' "$line" | cut -d$separateur -f1`

		#only happens at start: if keyWord hasn't been set, set it
		if [ -z "$keyWord" ]; then
			keyWord=`echo $courKeyWord | cut -d'#' -f1 | cut -d'%' -f1`
		#if we changed of ID, writes the translations of the old ID, then update it
		elif [ "$keyWord" != "`echo $courKeyWord | cut -d'#' -f1 | cut -d'%' -f1`" ]; then

			#for each languages, write into its file
			for j in `seq $langCount`; do
				printf '\t<string name="%s">"%s"</string>\n' "$keyWord" "${translation[`expr $j + 1`]}" >> /tmp/xmlFromTable/values-$j/strings.xml
			done

			#######OK, now we have to update our variables########

			#set the new keyWord for next lines
			keyWord=`echo $courKeyWord | cut -d'#' -f1 | cut -d'%' -f1`

			#empty the array
			for j in `seq $langCount`; do
				translation[`expr $j + 1`]=""
			done
		fi

		#multipart ID and it's a '#' part: meaning it needs a '\n\n' add.
		if [[ ! -z ${translation[2]} && `echo "$courKeyWord" | grep -e '#'` ]]; then
			for j in `seq $langCount`; do
				translation[`expr $j + 1 `]+='\n\n'
			done
		#multipart ID and it's a '%' part: meaning we add a space after the last dot
		elif [[ ! -z ${translation[2]} && `echo "$courKeyWord" | grep -e '%'` ]]; then
			for j in `seq $langCount`; do
				translation[`expr $j + 1 `]+=' '
			done
		fi

		#then we get the translations
		for j in `seq $langCount`; do
			jPlusUn=`expr $j + 1`

			#translation for this language
			singleT=`printf '%s' "$line" | cut -d$separateur -f $jPlusUn`

			#fill with english translation if there isn't one in this language (empty)
			if [ -z "$singleT" ]; then
				singleT=`printf '%s\n' "$line" | cut -d$separateur -f2`
			fi

			#and add them to the array
			translation[$jPlusUn]+=$singleT
		done

	done < $csvFILE

	#last line : ok, we need to write the last ID
	for j in `seq $langCount`; do
		printf '\t<string name="%s">"%s"</string>\n' "$keyWord" "${translation[`expr $j + 1`]}" >> /tmp/xmlFromTable/values-$j/strings.xml
	done

#########  4 : Rename dir by their lang name. #########
for i in `seq $langCount`; do
	echo "</resources>" >> /tmp/xmlFromTable/values-$i/strings.xml
	iPlusUn=`expr $i + 1`
	lang=`echo $firstLine | cut -d$separateur -f "$iPlusUn"`
	info "mv /tmp/xmlFromTable/values-$i $GAME_IS/res/values-$lang"

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

#########  5 : Clean tmp files.. #########
rm -r /tmp/xmlFromTable

rm $csvFILE
