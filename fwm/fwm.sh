#!/bin/ksh
# FWM


FLAGFILE=${HOME}/etc/fwm
LOGFILE=/tmp/fwm.log



F_ACTIVE=false
if [ -r ${FLAGFILE} ] ; then
  F_ACTIVE=true
fi

DATE=`date '+%y%m%d_%H%M:%S' `
echo ${DATE} > ${FLAGFILE}

MACH=`uname -n`
JID="${MACH}${$}"

if [ -w "${LOGFILE}" ] ; then {

echo "${JID}\tfwm ${DATE}"
echo "${JID}\tflagfile=${FLAGFILE}"

} >> ${LOGFILE} ; fi

${HOME}/etc/bin/forwardmail.cron
ES=$?

if [ -w "${LOGFILE}" ] ; then {
  echo "${JID}\tes=$?"
} >> ${LOGFILE} ; fi

if [ $F_ACTIVE = false ] ; then
  rm -f ${FLAGFILE}
fi



