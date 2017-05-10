#!/usr/extra/bin/ksh
# HELP

: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export LOCAL EXTRA

HELPFILE=${LOCAL}/etc/help.helpfile

if [[ -r "${HELPFILE}" ]] ; then
  if haveprogram shcat ; then
    shcat ${HELPFILE}
  else
    cat ${HELPFILE}
  fi
fi


