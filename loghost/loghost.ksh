#!/usr/bin/ksh
# LOGHOST


U=${1}
if [[ -z "${U}" ]] ; then
  U="-"
fi

userinfo ${U} loghost


