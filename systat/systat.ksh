#!/usr/extra/bin/ksh
# SYSTAT


: ${EXTRA:=/usr/extra}
export EXTRA

PROG=${EXTRA}/sbin/systatd

if haveprogram ${PROG} ; then
  ${PROG} "${@}"
fi


