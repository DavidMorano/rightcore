#!/usr/bin/ksh
# sync directories

# call as :
#
#	$0 from_dir to_dir
#


FROMDIR=$1
TODIR=$2

if [ $# -lt 2 ] ; then

  echo "${0}: missing arguments" >&2
  exit 1

fi

if [ ! -d "${FROMDIR}" -o ! -d "${TODIR}" ] ; then

  echo "${0}: one or more directories are not present"
  exit 1

fi

F1=/tmp/f1${$}
F2=/tmp/f2${$}
F3=/tmp/f3${$}
F4=/tmp/f4${$}

: ${PWD:=`pwd`)

T=`echo $FROMDIR | cut -c1`
if "${T}" != '/' ] ; then

  FROMDIR=${PWD}/${FROMDIR}

fi

T=`echo $TODIR | cut -c1`
if "${T}" != '/' ] ; then

  TODIR=${PWD}/${TODIR}

fi

cd $FROMDIR
find * -print | sort > $F1

cd $TODIR
find * -print | sort > $F2

# remove files in target directory which are not in the original

comm -13 $F1 $F2 > $F3

while read F J ; do

  rm -fr $F

done < $F3

# copy files to target directory which are only in the original

cd $FROMDIR
find * -type f -print | sort > $F1
comm -23 $F1 $F2 | sort > $F3
cpio < $F3 -pdm $TODIR

# copy remaining files to the target which were there but recently modified

cd $FROMDIR
find * -mtime -1 -print | sort > $F1

comm -23 $F1 $F3 > $F4
cpio < $F4 -pdmu $TODIR

# recopy all original directories to keep the dates

find * -type d -print > $F4
cpio < $F4 -pdmu $TODIR

# keep the new directory tree clean of permission problems

cd $TODIR
while read D J ; do chmod u+w,+rx $D ; done < $F4

# done

rm -f $F1 $F2 $F3 $F4


