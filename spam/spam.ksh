#!/usr/bin/ksh
# SPAM


: ${HOME:=$( userhome )}
: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${GNU:=/usr/add-on/gnu}
: ${EXTRA:=/usr/extra}
export HOME LOCAL NCMP GNU EXTRA

for PR in ${PRS} ; do
  if [[ -d ${PR} ]] ; then
    B=${PR}/fbin
    if [[ -d ${B} ]] ; then
      if [[ -n "${FPATH}" ]] ; then
        FPATH="${FPATH}:${B}"
      else
        FPATH=${B}
      fi
    fi
  fi
done
export FPATH

for PR in ${PRS} ; do
  pathadd PATH ${PR}/bin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done


BOGOFILTER_DIR=${GNU}/var/bogofilter
export BOGOFILTER_DIR

bogofilter -s "${@}" 


