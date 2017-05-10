#!/usr/bin/ksh
# LOGSESSION


U=${1}
if [[ -z "${U}" ]] ; then
  U="-"
fi

userinfo ${U} logsession


