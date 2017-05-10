#!/usr/bin/ksh
# MSGSUBJ

F=${1}
if [[ -r "${F}" ]] ; then
  ema -h subject ${F}
fi


