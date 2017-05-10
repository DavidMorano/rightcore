#!/usr/extra/bin/ksh
# GROPEO


: ${HOME:=$( userhome )}
: ${LOCAL:=/usr/add-on/local}
: ${TOOLS:=/usr/add-on/exptools}
: ${EXTRA:=/usr/extra}
export HOME LOCAL TOOLS EXTRA

PRS=" ${HOME} ${LOCAL} ${TOOLS} ${EXTRA} "

if [[ "${FPATH:0:1}" == ":" ]] ; then
  FPATH=${FPATH:1:200}
fi

for PR in ${PRS} ; do
  if [[ -d ${PR} ]] ; then
    FBIN=${PR}/fbin
    if [[ -d ${FBIN} ]] ; then
      if [[ -n "${FPATH}" ]] ; then
        FPATH="${FPATH}:${FBIN}"
      else
        FPATH="${FBIN}"
      fi
    fi
  fi
done
export FPATH

for PR in ${PRS} ; do
  pathadd PATH ${PR}/bin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done

for PR in ${PRS} ; do
  pathadd PATH ${PR}/sbin
done


DN=/dev/null
grope "${@}" < ${DN}
rm -f core


