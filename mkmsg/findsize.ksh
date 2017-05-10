#!/usr/bin/ksh
# FINDSIZE


# select your maximum size here (in bytes)
SIZE=10000
# for a "maximum" size, change the minux ('-') sign in front of
# the ${SIZE} argument below to '+'
#


TDIR=$1
if [[ -d ${TDIR} ]] ; then
   find ${TDIR} -type f -size +${SIZE}c -ls | {
     integer	sum=0
     while read A1 A2 A3 A4 A5 A6 S A8 A9 A10 FN ; do
       BN=${FN##*/}
       print -- ${S} ${FN} ${BN}
       (( sum += ${S} ))
     done
     print -- "sum=${sum}"
   }
fi



