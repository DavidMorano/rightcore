#!/usr/bin/ksh
# FWM



FLAGFILE=${HOME}/etc/fwm
LOGFILE=/tmp/fwm.log



F_ACTIVE=0
if [[ -r ${FLAGFILE} ]] ; then
  F_ACTIVE=1
fi

DATE=$( date '+%y%m%d_%H%M:%S' )
if [[ $F_ACTIVE -ne 0 ]] ; then
  print ${DATE} > ${FLAGFILE}
fi

if [[ -z "${NODE}" ]] ; then
  NODE=$( uname -n )
fi
JID="${NODE}${$}"

if [[ -w "${LOGFILE}" ]] ; then {
  print "${JID}\tfwm ${DATE}"
  print "${JID}\tflagfile=${FLAGFILE}"
} >> ${LOGFILE} ; fi

if [[ $F_ACTIVE -ne 0 ]] ; then

  ${PCS}/sbin/envset ${HOME}/sbin/forwardmail.cron +
  EX=$?

  if [[ -w "${LOGFILE}" ]] ; then {
    print "${JID}\tex=${EX}"
  } >> ${LOGFILE} ; fi

fi

if [[ $F_ACTIVE -eq 0 ]] ; then
  rm -f ${FLAGFILE}
fi



