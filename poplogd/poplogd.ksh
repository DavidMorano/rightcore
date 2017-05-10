#!/usr/bin/ksh
# POPLOGD


: ${LOCAL:=/usr/add-on/local}
: ${PCS:=/usr/add-on/pcs}

export LOCAL PCS


PATH=${PATH}:${LOCAL}/bin:${PCS}/bin
export PATH


SRCROOT=
RF_DEBUG=0

VERSION=0a

: ${NODENAME:=$( uname -n )}


: ${TMPDIR:=/tmp}
export TMPDIR

JOBFILE=$( pcsjobfile )

typeset -L14 JID=${JOBFILE}

JF=${TMPDIR}/${JOBFILE}
TF=${TMPDIR:=/tmp}/mi${RANDOM}a
HF=${TMPDIR:=/tmp}/mi${RANDOM}b
TF_TO=${TMPDIR:=/tmp}/mi${RANDOM}c
TF_CC=${TMPDIR:=/tmp}/mi${RANDOM}d
TF_BCC=${TMPDIR:=/tmp}/mi${RANDOM}e
RECIPFILE=${TMPDIR:=/tmp}/mi${RANDOM}f

cleanup() {
  rm -f $JF $TF $HF $TF_CC $TF_TO $TF_BCC $RECIPFILE
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

F_REMOVE=0
if [ $# -lt 1 ] ; then
  F_REMOVE=1
  MSG=${TF}
  cat > ${MSG}
else
  MSG=$1
  shift
fi


P=poplogd
#P=$( basename ${0} )


if [ ! -r "${MSG}" ] ; then
  echo "${P}: could not open message file \"${MSG}\"" >&2
  cleanup
  exit 1
fi

mailhead < $MSG > $HF
if [ ! -s "${MSG}" ] ; then
  echo "${P}: no headers in mail message file" >&2
  cleanup
  exit 1
fi


LOGFILE=${LOCAL}/log/${P}

: ${USERNAME:=$( username )}

DATE=$( date '+%y%m%d_%H%M:%S' )

logfile() {
  echo "${JID}\t${1}" >> ${LOGFILE}
}


# initial log entry
{

  echo "${JID}\t${DATE} ${P} ${VERSION}" 
  if [ -n "${FULLNAME}" ] ; then
    echo "${JID}\t${NODENAME}!${USERNAME} (${FULLNAME})"
  else
    echo "${JID}\t${NODENAME}!${USERNAME}"
  fi

} >> ${LOGFILE}

chmod go+rw ${LOGFILE} 2> /dev/null



MKMSG_OPTS=



PRIORITY=$( grep -i "^priority:" ${HF} | cut -d : -f 2-10 )
TAG=$( grep -i "^tag:" ${HF} | cut -d : -f 2-10 )




ES=0
msgbody ${MSG} | while read LINE ; do

  if [ -n "${LINE}" ] ; then
    logger -p $PRIORITY -t "${TAG}" $LINE
  fi

done


cleanup



