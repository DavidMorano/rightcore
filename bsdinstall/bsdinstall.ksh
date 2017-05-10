#!/usr/bin/ksh
# BSDINSTALL


PDIR=${0%/*}
A=${0##*/}
PNAME=${A%.*}



N=$#
if [[ "${N}" -lt 2 ]] ; then
  print -u2 "${PNAME}: not enough arguments\n" 
  exit 1
fi

NN=$( expr $N - 1 )
A=
C=1
while [[ $C -le $NN ]] ; do

  A="${A} ${1}"
  shift
  C=$( expr $C + 1 )

done

DNAME=${1}
if [[ -f "${DNAME}" ]] ; then
  DNAME=$( dirname $1 )
fi

find $A -type f -print | cpio -pdm $DNAME



