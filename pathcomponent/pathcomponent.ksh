#!/usr/bin/ksh
# PATHCOMPONENT


if [[ $# -lt 2 ]] ; then
  print -u2 "${0}: not enough arguments specified"
  exit 1
fi

pathenum ${1} | while read P J ; do
  if [[ "${P}" == "${2}" ]] ; then
    F=1
    break
  fi
done
EX=1
if [[ $F -ne 0 ]] ; then
  EX=0
fi

return $EX


