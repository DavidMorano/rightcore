#!/usr/bin/ksh
# MSGDIV

F=o
if [[ $# -gt 0 ]] ; then
  F=${1}
fi

COLS=76

if [[ -r "${F}" ]] ; then
  msgsubj ${F} > os
  msgbody ${F} | sanity | linefold -w ${COLS} | fold > ob
  MID=$( ema -h message-id ${F} )
  if [[ -n "${MID}" ]] ; then
    print -- "  <${MID}>" > oi
  else
    rm -f oi
  fi
else
  print -u2 -- "${0}: inaccesssible file"
fi


