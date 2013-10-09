#!/bin/bash

#from where are we calling it
fromWhereAmIBeingCalled=$PWD
#where the script is
whereAmI=$(cd "$(dirname "$0")" && pwd)
cd $whereAmI
#if we executed a linked script; go to the real one
if [  -h $0 ]; then
	whereAmI+=/$(dirname $(readlink $0))
	cd $whereAmI
fi
rootPath=$whereAmI"/../../.."

#import cool stuff
source ../coolStuff.sh

#how to use the script
export SAC_USAGE="$0 [options]"
export SAC_OPTIONS="\
all: print everything
I: print info
T: print todos
W: print warnings
E: print errors
C: desactivate colors"

export SAC_EXAMPLE="${green}$0 IWC will show only info and warnings, without color.{default_color}"

if [ $# != 1 ] || [ $(echo $1 | grep -- -h) ]; then
	usage_and_quit
fi

info=$(echo $1 | grep -i -e all -e I)
todo=$(echo $1 | grep -i -e all -e T)
warn=$(echo $1 | grep -i -e all -e W)
erro=$(echo $1 | grep -i -e all -e E)
nocolors=$(echo $1 | grep C)


if [ -z $info ] && [ -z "$warn" ] && [ -z "$erro" ]; then
	info "Warning: no LOG INFO, WARN and ERRO to print!(only std::cout codes).
Use 'all' option to enable all of them!" $orange
fi

while read LINE; do
	n=$(echo $LINE | grep INFO)
	t=$(echo $LINE | grep TODO)
	w=$(echo $LINE | grep WARN)
	e=$(echo $LINE | grep ERRO)

	# *** show info?
	if [ -n "$n" ]; then
		if [ -z $info ]; then
			continue
		fi

		if [ -z $nocolors ]; then
			info "$LINE$" $green
		else
			echo "$LINE"
		fi
	# *** show todos?
	elif [ -n "$t" ]; then
		if [ -z $todo ]; then
			continue
		fi

		if [ -z $nocolors ]; then
			info "$LINE$" $yellow
		else
			echo "$LINE"
		fi
	# *** show warnings?
	elif [ -n "$w" ]; then
		if [ -z $warn ]; then
			continue
		fi

		if [ -z $nocolors ]; then
			info "$LINE" $orange
		else
			echo "$LINE"
		fi
	# *** show errors?
	elif [ -n "$e" ]; then
		if [ -z $erro ]; then
			continue
		fi

		if [ -z $nocolors ]; then
			info "$LINE" $red
		else
			echo "$LINE"
		fi
	elif [ ! -z "$LINE" ]; then
		echo "(std::cout)$LINE"
	fi
done
