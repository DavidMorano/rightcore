#!/usr/extra/bin/ksh
# VOTD


: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export LOCAL EXTRA 


FPATH=${LOCAL}/fbin
if [[ ! -d "${FPATH}" ]] ; then
  FPATH=${EXTRA}/fbin
fi
export FPATH

DN=/dev/null

RF_DDASH=false
ARGVALUE=
ADD=
WIDTH=
PAS=
S=pos
for A in "${@}" ; do
  case "${A}" in
  -- )
    OS=${S}
    RF_DDASH=true
    ;;
  -w )
    OS=${S}
    S=width
    ;;
  -[0-9] | -[0-9][0-9] )
    OS=${S}
    ARGVALUE=$( print -- ${A} | cut -c 2-10 )
    ;;
  * )
    case ${S} in
    width )
      WIDTH=${A}
      ;;
    *)
      PAS="${PAS} ${A}"
      ;;
    esac
    S=${OS}
    ;;
  esac
done
#print -u2 "ADD=${ADD}"
#print -u2 "WIDTH=${WIDTH}"

OPTS=
if [[ -n "${WIDTH}" ]] ; then
  OPTS="${OPTS} -w ${WIDTH}"
fi
if [[ -n "${ARGVALUE}" ]] ; then
  OPTS="${OPTS} -${ARGVALUE}"
fi
OPTS="${OPTS} -o bookname=1,default"
if ${RF_DDASH} ; then
  OPTS="${OPTS} --"
fi

if whence pathadd > ${DN} ; then

  pathadd PATH ${LOCAL}/bin
  pathadd LD_LIBRARY_PATH ${LOCAL}/lib

  pathadd PATH ${EXTRA}/bin
  pathadd LD_LIBRARY_PATH ${EXTRA}/lib

  if whence bibleverse > /dev/null ; then
    bibleverse -t day ${OPTS} ${PAS}
  fi

fi


