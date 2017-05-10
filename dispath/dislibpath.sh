#!/bin/ksh
# DISLIBPATH
# display the current LD_LIBRARY_PATH

PROG_CUT=/bin/cut
PROG_FGREP=/bin/fgrep

F=0
I=1
echo ${LD_LIBRARY_PATH} | ${PROG_FGREP} ':' > /dev/null
if [ $? -eq 0 ] ; then

while [ $I -lt 100 ] ; do

A=`echo $LD_LIBRARY_PATH | cut -d: -f${I} `
if [ -n "${A}" ] ; then

  if [ $F -gt 0 ] ; then echo "**CURRENT DIRECTORY**" ; fi

  echo $A
  F=0

else

  F=`expr $F + 1 `
  if [ $F -gt 1 ] ; then exit 0 ; fi

fi

I=`expr $I + 1 `

done

else

  echo $LD_LIBRARY_PATH

fi

exit 0


