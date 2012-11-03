#!/bin/bash

if [[ $# != 1 || `echo $1 | grep -- -h` ]]; then
	echo "Usage: $0 options"
	echo "	- all: print all"
	echo "	- I: print info"
	echo "	- W: print warnings"
	echo "	- E: print errors"
	echo "	- C: desactivate colors"
	echo "Example : \"$0 I-W-C\" will show only info and warnings, without color."
	exit -1
fi

info=`echo $1 | grep all`
info+=`echo $1 | grep I`

warn=`echo $1 | grep all`
warn+=`echo $1 | grep W`

erro=`echo $1 | grep all`
erro+=`echo $1 | grep E`

nocolors=`echo $1 | grep C`

if [[ -z $info && -z "$warn" && -z "$erro"	]]; then
	echo -e "\033[1m\E[31mWarning, no LOG INFO, WARN and ERRO to print!(only std::cout codes)"
	echo -e "Use 'all' option to enable all of them!\033[0m"
fi
	
while read LINE; do
	n=`echo $LINE | grep INFO`
	w=`echo $LINE | grep WARN`
	e=`echo $LINE | grep ERRO
	`
	# *** show info?
	if [ -n "$n" ]; then
		if [ -z $info ]; then
			continue
		fi
		
		if [ -z $nocolors ]; then
			echo -e "\033[1m\E[32m$LINE\033[0m"; tput sgr0
		else 
			echo -e "$LINE"
		fi
	# *** show warnings?
	elif [ -n "$w" ]; then
		if [ -z $warn ]; then
			continue
		fi
		
		if [ -z $nocolors ]; then
			echo -e "\033[1m\E[33m$LINE\033[0m"; tput sgr0
		else 
			echo -e "$LINE"
		fi
	# *** show errors?
	elif [ -n "$e" ]; then
		if [ -z $erro ]; then
			continue
		fi
		
		if [ -z $nocolors ]; then
			echo -e "\033[1m\E[31m$LINE\033[0m"; tput sgr0
		else 
			echo -e "$LINE"
		fi
	elif [ ! -z "$LINE" ]; then
		echo -e "(std::cout)$LINE"
	fi
done
	
exit 0
