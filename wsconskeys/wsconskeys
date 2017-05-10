#!/usr/bin/ksh
# WSCONSKEYS


RF_WSCONS=false
WNCONSKEYS_KEYS=/etc/wscons.keys

if [[ -r ${WNCONSKEYS_KEYS} ]] ; then
  strconf | while read M J ; do
    if [[ "${M}" == "wc" ]] ; then
      RF_WSCONS=true
      break
    fi
  done
  if ${RF_WSCONS} ; then
    loadkeys ${WNCONSKEYS_KEYS}
  fi
fi



