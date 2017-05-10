#!/usr/extra/bin/ksh
# MKQOTD (Make Quote-Of-The-Day)

# program version here
PV=0


: ${HOME:=$( userhome )}
: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${EXTRA:=/usr/extra}
export HOME LOCAL NCMP EXTRA

: ${GAMESBIN:=/usr/games}

PRS=" ${HOME} ${LOCAL} ${NCMP} ${EXTRA} "

if [[ "${FPATH:0:1}" == ":" ]] ; then
  FPATH=${FPATH:1:200}
fi

for PR in ${PRS} ; do
  if [[ -d ${PR} ]] ; then
    FBIN=${PR}/fbin
    if [[ -d ${FBIN} ]] ; then
        FPATH="${FPATH}:${FBIN}"
    fi
  fi
done
export FPATH

for PR in ${PRS} ; do
  pathadd PATH ${PR}/bin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done

PN=${0##*/}
DN=/dev/null
PR=${LOCAL}

QSDNAME=${PR}/spool/maintqotd

pathadd PATH ${GAMESBIN}

if [[ ! -d ${QSDNAME} ]] ; then
  mkdir ${QSDNAME}
fi

TF=/tmp/${PN}${$}

function cleanup {
  rm -f ${TF}
}

trap 'cleanup;exit' 1 2 3 15 16 17

LOGFILE=${PR}/log/${PN}
logfile -c ${LOGFILE} -n ${PN}:${PV} -s 10m |&

function logprint {
  typeset V="${1}"
  if [[ -n "${V}" ]] ; then
    print -p -- "${V}"
  fi
}

if [[ -d ${QSDNAME} ]] ; then
  true > ${TF}
  while [[ ! -s ${TF} ]] ; do
    fortune > ${TF}
    FS=$( fsize ${TF} )
    logprint "fsize=${FS}"
  done
  shcat ${TF}
else
  logprint "inaccessible spool directory"
fi

cleanup


