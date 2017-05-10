#!/usr/extra/bin/ksh
# CBB

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


A=${0##*/}
PN=${A%.}

if [[ "${#}" -gt 0 ]] ; then
  SRC=${1}
  if [[ -n "${SRC}" ]] ; then
    NSRC=N${SRC}
    cp -p ${SRC} O${SRC}
    cb ${SRC} | cci > ${NSRC}
    if [[ -s ${NSRC} ]] ; then
      cp ${NSRC} ${SRC}
      rm -f ${NSRC}
    fi
  else
    print -u2 "${PN}: NUL argument specified"
  fi
else
  print -u2 "${PN}: no argument specified"
fi


