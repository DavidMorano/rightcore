#!/usr/extra/bin/ksh
# PLAN (program)

  if [[ -z "${1}" ]] ; then
    : ${USERNAME:=$( username )}
    rfinger ${USERNAME}
  else
    rfinger "${@}"
  fi


