#!/usr/extra/bin/ksh
# NETSTATD

PN=netstatd
DN=/dev/null

: ${HOME:=$( /usr/extra/bin/userhome )}
: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export HOME LOCAL EXTRA

PRS=" ${HOME} ${PCS} ${LOCAL} ${NCMP} ${EXTRA} "

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
        FPATH=${FBIN}
      fi
    fi
  fi
done
export FPATH

for PR in ${HOME} ${PRS} ; do
  pathadd PATH ${PR}/bin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done


if kshbi shcat | fgrep YES > ${DN} ; then
  builtin -f ${LOCAL}/lib/liblkcmd.so shcat 
fi

BANFILE=${LOCAL}/etc/${PN}/banner
if [[ -r ${BANFILE} ]] ; then
  BANMSG="$( < ${BANFILE} )"
else
  BANMSG="RightCore"
fi

if haveprogram bandate ; then
  banner "${BANMSG}" | bandate
else
  banner "${BANMSG}"
fi
shcat -Q admin~logwelcome­netstat
print -- "The RightCore network is UP."


