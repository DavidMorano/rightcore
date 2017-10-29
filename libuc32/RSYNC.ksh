#!/usr/bin/ksh
# RSYNC


RHOST=rcf
RDIR=src/libuc32


if [[ $# -gt 0 ]] ; then
  RHOST=${1}
fi

USERNAME=morano
export USERNAME

RSYNC_RSH=rsh
export RSYNC_RSH

rsync --rsh=rsh -rltH README Makefile *.[ch] ${RHOST}:${RDIR}




