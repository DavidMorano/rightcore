#!/bin/ksh
# BSDINSTALL

N=$#
if [ "${N}" -lt 2 ] ; then

  echo "${0}: not enough arguments\n" >&2
  exit 1

fi

NN=`expr $N - 1 `
A=""
C=1
while [ $C -le $NN ] ; do

  A="${A} ${1}"
  shift
  C=`expr $C + 1 `

done

B=$1
if [ -f "${1}" ] ; then

  B=`basename $1 `

fi

find $A -type f -print | cpio -pdm $B


