#!/usr/bin/ksh
# MKTMPUSER


: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export LOCAL EXTRA


FPATH=${LOCAL}/fbin
if [[ ! -d "${FPATH}" ]] ; then
  FPATH=${EXTRA}/fbin
fi
export FPATH


autoload mktmpuser

mktmpuser "${@}"



