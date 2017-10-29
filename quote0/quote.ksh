#!/usr/bin/ksh
# QUOTE


: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
export LOCAL NCMP


IDB=${LOCAL}/var/txtindexes/fortune 
QD=${NCMP}/share/fortune

mkquery -db ${IDB} ${@} | mktagprint -b ${QD}



