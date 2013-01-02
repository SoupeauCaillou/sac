#!/bin/sh

#inspired from http://www.intuitive.com/wicked/showscript.cgi?011-colors.sh
initializeANSI()
{
  esc=""

	reset="[0m"
	red="[1m[31m"
	green="[1m[32m"
	yellow="[1m[33m"

}

if [ $# != 1 ] || [ `echo $1 | grep -- -h` ]; then
	echo "Usage: $0 options"
	echo "	- all: print all"
	echo "	- I: print info"
	echo "	- W: print warnings"
	echo "	- E: print errors"
	echo "	- C: desactivate colors"
	echo "Example : \"$0 I-W-C\" will show only info and warnings, without color."
	exit -1
fi

info=`echo $1 | grep -i -e all -e I`

warn=`echo $1 | grep -i -e all -e W`

erro=`echo $1 | grep -i -e all -e E`

nocolors=`echo $1 | grep C`

initializeANSI

if [ -z $info ] && [ -z "$warn" ] && [ -z "$erro" ]; then
	echo "${red}Warning, no LOG INFO, WARN and ERRO to print!(only std::cout codes)"
	echo "Use 'all' option to enable all of them!${reset}"
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
			echo "${green}$LINE${reset}"
		else
			echo "$LINE"
		fi
	# *** show warnings?
	elif [ -n "$w" ]; then
		if [ -z $warn ]; then
			continue
		fi

		if [ -z $nocolors ]; then
			echo "${yellow}$LINE${reset}"
		else
			echo "$LINE"
		fi
	# *** show errors?
	elif [ -n "$e" ]; then
		if [ -z $erro ]; then
			continue
		fi

		if [ -z $nocolors ]; then
			echo "${red}$LINE${reset}"
		else
			echo "$LINE"
		fi
	elif [ ! -z "$LINE" ]; then
		echo "(std::cout)$LINE"
	fi
done

exit 0
