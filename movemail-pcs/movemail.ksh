#!/usr/bin/ksh
# MOVEMAIL


VERSION=0


FROMFILE=$1
TOFILE=$2

: ${PCS:=/usr/add-on/pcs}
export PCS

LOGFILE=${PCS}/log/movemail

MACH=`uname -n`
LOGID=`echo "${MACH}${$}     " | cut -c 1-14 `

logprint() {
  echo "${LOGID} ${*}" >> ${LOGFILE}
}


DATE=` date '+%y%m%d_%T' `
logprint "${DATE} movemail ${VERSION}"

: ${NAME:=`logname - name`}
: ${LOGNAME:=`logname`}

if [ -n "${NAME}" ] ; then
  logprint "${MACH}!${LOGNAME} (${NAME})"
else
  logprint "${MACH}!${LOGNAME}"
fi


TF="${TMPDIR:=/tmp}/mm${$}"

cleanup() {
  rm -f $TF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


if [ -n "${TOFILE}" ] ; then
  getmail -r $TF -m $TOFILE
fi

if [ -s $TF ] ; then
  BYTES=`cat ${TF} `
  logprint bytes=${BYTES}
fi


cleanup


