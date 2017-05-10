#!/usr/extra/bin/ksh
# TELSERVD


: ${HOME:=$( userhome )}
: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export HOME LOCAL EXTRA

PRS=" ${HOME} ${LOCAL} ${EXTRA} "

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


P_EXECNAME=${EXTRA}/bin/execname


TELSERV_ERRORFILE=/tmp/telserv
export TELSERV_ERRORFILE

TELSERV_DEBUGFILE=/tmp/telserv.d
export TELSERV_DEBUGFILE

cp /dev/null ${TELSERV_DEBUGFILE}

EX=0
if [[ ! -x "${P_EXECNAME}" ]] ; then
  ${P_EXECNAME} ${EXTRA}/bin/telserv + "${@}"
else
  EX=1
fi

return ${EX}


