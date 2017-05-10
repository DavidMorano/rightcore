#!/usr/bin/ksh
# FINGERWEBCOUNTS


: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export LOCAL EXTRA

: ${WWWCOUNTERS:=/var/www/counters}
export WWWCOUNTERS


FPATH=${FPATH}:${LOCAL}/fbin:${EXTRA}/fbin
export FPATH

pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/bin
export PATH LD_LIBRARY_PATH


A="${1}"
DBS=$( fieldwords "${A}" )

#print -u2 ${DBS}

for DB in ${DBS} ; do
  DBDIR=${WWWCOUNTERS}/${DB}
  if [[ -r "${DBDIR}" ]] ; then
    print site=${DB}
    webcounter -db ${DBDIR} -l
  fi
done



