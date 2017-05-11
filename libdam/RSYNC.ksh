#!/usr/bin/ksh
# RSYNC


RHOST=jig
RDIR=src/libdam32


if [[ $# -gt 0 ]] ; then
  RHOST=${1}
fi

USERNAME=morano
export USERNAME

RSYNC_RSH=rsh
export RSYNC_RSH

rsync --rsh=rsh -rtlH README Makefile *.[ch] ${RHOST}:${RDIR}




