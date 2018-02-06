#!/usr/bin/ksh
# MAKENEWER


if [ $# -lt 2 ] ; then
  echo "${0}: not enough arguments" >&2
  exit 1
fi

SRC=$1
DSTDIR=$2
if [[ ! -d "${DSTDIR}" ]] ; then
  print -u 2 "${0}: destination is not a directory"
  exit 1
fi

DST=${DSTDIR}/${SRC}
if [ $SRC -nt $DST ] ; then
  find $SRC -print | cpio -pdm $DST
fi



