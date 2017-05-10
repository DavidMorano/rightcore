#!/bin/ksh
# DMAIL


: ${LOCAL:=/usr/add-on/pcs}
: ${PCS:=/usr/add-on/pcs}
export LOCAL PCS


PROG_DMAIL=/usr/extra/sbin/dmail.s5
if [[ ! -x $PROG_DMAIL ]] ; then
  PROG_DMAIL=${PCS}/bin/dmail
fi

EXTRAOPTS="-o divert"
${PROG_DMAIL} ${EXTRAOPTS} "${@}"



