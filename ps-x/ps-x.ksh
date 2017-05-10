#!/usr/bin/ksh
# PS-X


P=ps
OPTS=

if [ -d /usr/sbin ] ; then
  : ${USERNAME:=$( username )}
  if [[ -n "${USERNAME}" ]] ; then
    OPTS="-f -U ${USERNAME} -o \"pid ppid tty time args\""
  else
    OPTS="-f"
  fi
else
  OPTS="-x"
fi

eval exec ${P} ${OPTS} "${@}"



