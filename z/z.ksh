#!/usr/bin/ksh
# Z

RF_CHMOD=false

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


