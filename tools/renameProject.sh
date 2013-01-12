#!/bin/bash

#how to use the script
export USAGE="$0 pathOfGame OldName NewName [--preview]"
export OPTIONS="--preview : don't apply changes."
export EXAMPLE="$0 /tmp/heriswap Heriswap Prototype"
#where the script is
whereAmI=`cd "$(dirname "$0")" && pwd`
#import cool stuff
source $whereAmI/coolStuff.sh

######### 0 : Check arguments. #########
   #check arg count
   if [ "$#" -lt 3 ]; then
      error_and_usage_and_quit "Need 3+ args [and NOT '$#']."
   fi

   #check that the gamepath does exist
   gamePath="$1"
   if [ ! -e "$gamePath" ]; then
      error_and_usage_and_quit "The game path '$gamePath' doesn't exist ! Abort."
   fi

   # Remove potentials spaces in names and make lower / upper case.
   oldNameLower=`echo $2 | tr -d " " | sed 's/^./\l&/'`
   newNameLower=`echo $3 | tr -d " " | sed 's/^./\l&/'`

   oldNameUpper=`echo $2 | tr -d " " | sed 's/^./\u&/'`
   newNameUpper=`echo $3 | tr -d " " | sed 's/^./\u&/'`

   #check that the new gamepath does NOT exist
   newGamePath=$(cd $gamePath/.. && pwd)"/$newNameLower"
   if [ -e "$newGamePath" ]; then
      error_and_usage_and_quit "$newGamePath already exist ! Please delete it to continue! Abort."
   fi

   #look if we have to apply change
   if [ $# = 4 ] && [ $4 = "--preview" ]; then
      applyChanges=""
      info "Changes won't be applied"
   else
      applyChanges="yes"
   fi





   info "
   INFO: the name you chose is '$newNameUpper'
   ********************************************************************************
   * WARNING: don't use a common name else you could overwrite wrong files        *
   * Eg: don't call it 'test' or each files TestPhysics.cpp,... will be renamed!  *
   ********************************************************************************
   " $blue
######### 2 : Wait for confirmation to continue #########
   echo "Continue ? (y/N)"
   read confirm
   if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
      info "Aborted" $red
      exit
   fi

   #now go to the gamePath
   cd $gamePath
   #make it absolute
   gamePath=$PWD

######### 3 : Rename files and dir #########
   #add dir to ignore here. Ex :my-dir-to-ignore/
   IGNOREDIR=".git build bin sac gen libs obj"


   IGNOREDIR=$(echo $IGNOREDIR | sed 's| |/ -e /|g')
   IGNOREDIR=" -v -e /$IGNOREDIR/"
   #IGNOREDIR looks like "-v -e /.git/ -e /build/ -e /bin/ -e /sac/ -e /gen/ -e /libs/" now

   #1) directory in reverse order (rename protype/ before prototype/prototype.cpp )
   directories=$(find . -type d -iname "*$oldNameLower*" | grep $IGNOREDIR)

   while [ ! -z "$directories" ]; do
      first=$(echo $directories | cut -d" " -f1)
      new=${first/$oldNameLower/$newNameLower}
      new=${new/$oldNameUpper/$newNameUpper}

      info "Renaming directory $first to $new"
      if [ ! -z $applyChanges ]; then
         git mv $first $new
      else
         break
      fi

      directories=$(find . -type d -iname "*$oldNameLower*" | grep $IGNOREDIR)
   done


   #2) files
   for file in $(find . -type f -iname "*$oldNameLower*" | grep $IGNOREDIR); do
      new=${file/$oldNameLower/$newNameLower}
      new=${new/$oldNameUpper/$newNameUpper}

      info "Renaming file $file to $new"

      if [ ! -z $applyChanges ]; then
         git mv $file $new
      fi
   done

   #3) in files
   if [ ! -z $applyChanges ]; then
      for file in $(find . -type f | grep $IGNOREDIR | cut -d/ -f2-); do
         sed -i "s|$oldNameLower|$newNameLower|g" $file
         sed -i "s|$oldNameUpper|$newNameUpper|g" $file
      done
   fi


######### 4 : Clean build dir [optionnal] #########
   info "Want to clean build directory ? (y/N)" $blue
   read confirm
   if [[ "$confirm" == "y" || "$confirm" == "Y" ]]; then
      info "rm -rf build && cp -r sac/tools/build ."
      if [ ! -z $applyChanges ]; then
         rm -rf build && cp -r sac/tools/build .
      fi
   fi

######### 5 : rename the root dir [optionnal] #########
   info "Want to rename the root dir from '$gamePath' to '$newGamePath' ? (y/N)" $blue
   read confirm
   if [[ "$confirm" == "y" || "$confirm" == "Y" ]]; then
      info "mv $gamePath $newGamePath"
      if [ ! -z $applyChanges ]; then
         mv $gamePath $newGamePath
      fi
   fi

