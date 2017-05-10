#!/usr/bin/ksh
# LOGID


U=${1}
if [[ -z "${U}" ]] ; then
  U="-"
fi

userinfo ${U} logid


