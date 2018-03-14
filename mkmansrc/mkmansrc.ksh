#!/usr/extra/bin/ksh
# GETMANSRC

UBIN=/usr/bin
if [[ -r "${1}" ]] ; then
  B=${1##*/}
  R=${B%.*}
  /usr/lib/sgml/sgml2roff ${1} | ${UBIN}/tbl | ${UBIN}/eqn > ${R}.man
else
  print -u2 -- "${0}: file not readable"
fi


