#!/usr/bin/ksh
# PCSMOTD

ADMIN=${1}
if [[ -z "${ADMIN}" ]] ; then ADMIN=pcs ; fi
motd ${ADMIN}


