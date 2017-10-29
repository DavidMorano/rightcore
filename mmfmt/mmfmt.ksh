#!/usr/bin/ksh
# MFMT

W=${1}
if [[ -z "${W}" ]] || [[ "${W}" -eq 0 ]] ; then W="72" ; fi

OPTS=
if [[ -n "${W}" ]] ; then
  OPTS="${OPTS} -w ${W}"
fi

fmt ${OPTS}


