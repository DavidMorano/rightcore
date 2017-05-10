#!/usr/bin/ksh
# LOGLINE


U=${1}
if [[ -z "${U}" ]] ; then
  U="-"
fi

userinfo ${U} logline


