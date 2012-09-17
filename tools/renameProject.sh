#!/bin/sh

#test nb of args
if [ "$#" -ne 3 ]; then
	echo "Need 2 args :"
	echo "\tsh renameProject.sh pathOfGame oldName newName"
	echo "Abort."
	exit
fi

path="$1"
if [ ! -e "$path" ]; then
	echo "$path doesn't exist ! Abort."
	echo "\tUsage : sh renameProject.sh pathOfGame oldName newName"
	exit
fi

#remove potentials spaces in names:
oldName=`echo $2 | tr -d " "`
newName=`echo $3 | tr -d " "`

#now go to the path
cd $path

#rename files
#~ls -R . | grep ".java" 
#rename in files
#probleme avec git ??
#~find . -type f -print0 | xargs -0 sed -i 's/$oldName/$newName/g'

#rename the root dir
cd ..
mv `echo $path | awk -F/ '{print $NF}'` $newName
