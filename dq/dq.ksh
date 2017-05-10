#!/usr/bin/ksh
# DQ


DN=/dev/null
if whence loghost > ${DN} ; then

    if [[ -n "${BASETERMDEV}" ]] && [[ -n "${BASETERM}" ]] ; then
      s -T ${BASETERM} -of ${BASETERMDEV} -o clear=0,date=1
    else
      s -o clear=0,date=1
    fi

fi


