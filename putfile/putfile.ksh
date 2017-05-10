#!/bin/ksh
# PUTFILE


if [[ -z "${1}" ]] ; then
  echo "${0}: no file name was specified" >&2
  exit 1
fi

eval cp /dev/null $1
eval append $1


