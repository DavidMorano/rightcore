#!/usr/bin/ksh
# MSCHECKUP



CN=ece
if [[ -n "${1}" ]] ; then
  CN=${1}
fi


: ${TMPDIR:=/tmp}
export TMPDIR

LOCKDIR=${TMPDIR}

LKFILE=${LOCKDIR}/${PN}.lk


function cleanup {
  rm -f ${LKFILE}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

lkfile ${LKFILE}
EX=$?
if [[ $EX -eq 0 ]] ; then


integer maxage age f=0

(( maxage = 24 * 60 ))

pingstat -p ${CN} | while read S N J ; do

  print -u2 "S=>${S}< N=>${N}< J=>${J}<"

  f=0
  AGE=$( msage $N )
  if [[ -z "${AGE}" ]] || [[ $AGE == "*NA*" ]] ;then
    (( f = 1 ))
  fi
  if (( (! f) )) ; then
    age=${AGE}
    if (( age > maxage )) ; then
      (( f = 1 ))
    fi
  fi

  if (( f )) ; then

      case $S in

      U )
        cex -n $N msu -d -q -caf
        ;;

      esac

  fi

done 2> /dev/null


fi


cleanup



