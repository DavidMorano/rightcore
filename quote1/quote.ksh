#!/usr/extra/bin/ksh
# QUOTE

IDXNAME=fortune

: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
export LOCAL NCMP

DBDIR=${NCMP}/share/fortune 
IDXDIR=${LOCAL}/var/txtindexes
IDX=${IDXDIR}/${IDXNAME}
if [[ -d ${IDXDIR} ]] ; then
  mkquery -db ${IDX} "${@}" | mktagprint -b ${DBDIR}
else
  print -u2 -- "${0}: IDX not present"
fi


