#!/usr/bin/ksh
# RLOAD


RHOST=rcg
RDIR=src
RUSER=dam


if [[ $# -gt 0 ]] ; then
  RHOST=${1}
fi


: ${HOME:=$( userdir )}
export HOME


RF_AVAIL=false
if pingstat -q $RHOST ; then
  RF_AVAIL=true
fi

if [[ ${RF_AVAIL} == 'false' ]] ; then
  print -u2 "${0}: remote host is not aviailable"
  exit 1
fi


RSYNC_RSH=rsh
export RSYNC_RSH

VARTMP=${HOME}/var/spool/tmp
if [[ ! -d ${VARTMP} ]] ; then
  mkdir -p ${VARTMP}
fi

TD=${VARTMP}/rs${$}
mkdir -p ${TD}

cleanup() {
  rm -fr ${TD}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

{
  find *.txt -print
  find * -type f -name '*Makefile' -print
  find * -type f -name '*makeit' -print
  find * -type f -name '*README*' -print
  find * -type f -name '*.[ch]' -print
  find * -type f -name '*.help' -print
} | ocpio -oc > ${TD}/load.ca

if [[ -d ${TD} ]] ; then (
  cd ${TD}
  USERNAME=${RUSER} rcp -p load.ca ${RHOST}:${RDIR}
) fi

cleanup


