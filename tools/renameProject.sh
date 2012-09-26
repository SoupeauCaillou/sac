#!/bin/bash

# ça peut bug si on utilise un nom utilisé ailleurs (ex : "test", c'est pas que le nom du projet …) /!\
# utilisation d'un caractere spécial pour etre sur ? _name ou ??

#test nb of args
if [ "$#" -ne 3 ]; then
	echo "Need 3 args :"
	echo "\tsh renameProject.sh pathOfGame OldName NewName"
	echo "\tsh renameProject.sh /tmp/heriswap Heriswap Prototype"
	echo "Abort."
	exit
fi

#remove potentials spaces in names and make lower / upper case (Heriswap / heriswap):
oldNameLower=`echo $2 | tr -d " " | sed 's/^./\l&/'`
newNameLower=`echo $3 | tr -d " " | sed 's/^./\l&/'`

oldNameUpper=`echo $2 | tr -d " " | sed 's/^./\u&/'`
newNameUpper=`echo $3 | tr -d " " | sed 's/^./\u&/'`

#vérifie que le path de départ existe
path="$1"
if [ ! -e "$path" ]; then
	echo "$path doesn't exist ! Abort."
	echo "\tUsage : sh renameProject.sh pathOfGame oldName newName"
	exit
fi

#vérifie que le path d'arrivée N'EXISTE PAS
newpath=`cd $path && cd .. && pwd`/$newNameLower
if [ -e "$newpath" ]; then
	echo "$newpath already exist ! Please delete it to continue! Abort."
	exit
fi

echo "***********************************************************************************"
echo "* WARNING : don't use a common name else you could overwrite wrong files          *"
echo "* Eg : don't call it \"test\" or each files TestPhysics.cpp,... will be renamed!    *"
echo "* Continue ? (y/N) ?                                                              *"
echo "***********************************************************************************"
read continue
if [[ "$continue" != "y" && "$continue" != "Y" ]]; then
    echo "abort"
    exit
fi

#now go to the path
cd $path

#rename files and dir
#first directory in reverse order (rename protype/ before prototype/prototype.cpp )
todo=""
for i in `find . -type d | grep -i $oldNameLower`; do
    new=${i/$oldNameLower/$newNameLower}
    new=${new/$oldNameUpper/$newNameUpper}
    
    echo "Renaming directory $i to $new"
    todo="mv $i $new; "$todo
done
`$todo`
#then files
for i in `find . -type f  | grep -i $oldNameLower`; do
    new=${i/$oldNameLower/$newNameLower}
    new=${new/$oldNameUpper/$newNameUpper}
    
    echo "Renaming file $i to $new"
    mv $i $new
done

#rename in files (ignoring files in sac/ )
for i in `find . -type f | grep -v "/sac/" | cut -d/ -f2-`; do
	sed -i 's/$oldNameLower/$newNameLower/g' $i
	sed -i 's/$oldNameUpper/$newNameUpper/g' $i
done

#rename the root dir
cd ..
echo "Renaming root dir : mv `echo $path | tr -d /` $newNameLower"
mv `echo $path | tr -d /` $newNameLower
