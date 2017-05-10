#!/usr/extra/bin/ksh
# CBIT

RF_DEBUG=false

: ${HOME:=$( userhome )}
: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${GNU:=/usr/add-on/gnu}
: ${EXTRA:=/usr/extra}
export HOME LOCAL NCMP GNU EXTRA


PRS=" ${LOCAL} ${HOME} ${EXTRA} "
for PR in ${PRS} ; do
  if [[ -d ${PR} ]] ; then
    FBIN=${PR}/fbin
    if [[ -d ${FBIN} ]] ; then
        FPATH="${FPATH}:${FBIN}"
    fi
  fi
done
export FPATH


pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib

pathadd PATH ${NCMP}/bin
pathadd LD_LIBRARY_PATH ${NCMP}/lib

pathadd PATH ${GNU}/bin
pathadd LD_LIBRARY_PATH ${GNU}/lib

pathadd PATH ${EXTRA}/bin
pathadd LD_LIBRARY_PATH ${EXTRA}/lib


A=${0##*/}
PN=${A%.}


TF=${TMPDIR:=/tmp}/cbit${RANDOM}${$}

function cleanup {
  rm -f ${TF}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


for F in "${@}" ; do
  if [[ -w "${F}" ]] ; then
    OFILE=O${F}
    rm -f ${OFILE}
    makecp ${F} ${OFILE}
    cb ${F} | cci > ${TF}
    if [[ -s "${TF}" ]] ; then
      cp ${TF} ${F}
    fi
  else
    print -u2 "${PN}: could not open file=${F}" 
  fi
done

cleanup


