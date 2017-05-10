#!/usr/extra/bin/ksh
# COTD


: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export LOCAL EXTRA 


FPATH=${LOCAL}/fbin
if [[ ! -d "${FPATH}" ]] ; then
  FPATH=${EXTRA}/fbin
fi
export FPATH


RF_DDASH=false
ADD=
WIDTH=
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
    ADD="${A}"
    ;;
  n[0-9] | n[0-9][0-9] )
    OS=${S}
    ADD="$( print ${A} | cut -c 2-10 )"
    ;;
  p[0-9] | p[0-9][0-9] )
    OS=${S}
    ADD="-$( print ${A} | cut -c 2-10 )"
    ;;
  n )
    OS=${S}
    ADD=1
    ;;
  p )
    OS=${S}
    ADD="-1"
    ;;
  * )
    case ${S} in
    width )
      WIDTH=${A}
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

if whence pathadd > /dev/null ; then

  pathadd PATH ${LOCAL}/bin
  pathadd LD_LIBRARY_PATH ${LOCAL}/lib

  pathadd PATH ${EXTRA}/bin
  pathadd LD_LIBRARY_PATH ${EXTRA}/lib

  if whence commandment > /dev/null ; then
    integer N
    D=$( date '+%d' )
    integer n=${D}
    (( n = (n % 10) ))
    if [[ -n "${ADD}" ]] ; then
      (( n += (( ${ADD} + 100 ) % 10) ))
    fi
    (( N = ((n == 0) ? 10 : n) ))
    commandment ${OPTS} ${N} 
  fi

fi



