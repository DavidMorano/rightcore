#!/usr/bin/ksh
# MAILREQUEST


P=mailrequest


: ${LOCAL:=/usr/add-on/local}
: ${PCS:=/usr/add-on/pcs}
export LOCAL PCS

PATH=${PATH}:${PCS}/bin
export PATH


SERVICE=fwm

RNODES="uri "


F_POP=0
if [[ -n "${1}" ]] ; then
  F_POP=1
fi


STAMP=${PCS}/spool/timestamps/${P}
if fileolder $STAMP 5m ; then :
  touch $STAMP
  chmod 666 $STAMP
else
  exit 0
fi 2> /dev/null


: ${TMPDIR:=/tmp}

FN=${TMPDIR}/rm${RANDOM}.${SERVICE}
DATE=$( date '+%y%m%d_%H%M:%S' )

echo $DATE > ${FN}

for RNODE in $RNODES ; do
  uucp -r -C ${FN} ${RNODE}!~/uuts/
done

if [ $F_POP -ne 0 ] ; then
  for RNODE in $RNODES ; do
    uupoll ${RNODE} &
  done
fi

rm -f $FN



