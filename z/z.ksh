#!/usr/bin/ksh
# Z


RF_CHMOD=false
RF_DATE=false
if [[ "${TERM}" == "vt520" ]] ; then
  RF_DATE=true
fi


for F in "${@}" ; do
  RF_CHMOD=false
  if [[ -n "${F}" ]] ; then
    if [[ ! -w ${F} ]] ; then
      rm -f ${F}
      RF_CHMOD=true
    fi
    cp /dev/null ${F}
    if ${RF_CHMOD} ; then
      chmod o+w ${F}
    fi
  fi
done

if ${RF_DATE} && whence s > /dev/null 2>&1 ; then
  s -o clear=0
fi


