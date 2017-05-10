#!/usr/bin/ksh
# PS-SIZE


U=${1}
if [[ ${#} -lt 1 ]] ; then
  : ${USERNAME:=$( username )}
  U=${USERNAME}
else
  if username $U -q ; then :
  else
    print -u2 "${0}: invalid username"
    exit 1
  fi
fi

exec ps -U ${U} -o "pid vsz rss time args" 



