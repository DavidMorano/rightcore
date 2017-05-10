#!/usr/bin/ksh
# D

date "${@}"

DN=/dev/null
if whence loghost > ${DN} ; then
    if [[ -n "${BASETERMDEV}" ]] && [[ -n "${BASETERM}" ]] ; then
      s -T ${BASETERM} -of ${BASETERMDEV} -o clear=0
    else
      s -o clear=0
    fi
fi

#sync &


