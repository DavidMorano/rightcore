#!/usr/extra/bin/ksh
# HOLS

: ${HOME:=$( userhome )}
: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export LOCAL EXTRA 

PRS=" ${HOME} ${LOCAL} ${EXTRA} "

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


RF_DDASH=false
ADDS=
WIDTH=
S=adds
for A in "${@}" ; do
  case "${A}" in
  -- )
    RF_DDASH=true
    ;;
  -w )
    OS=${S}
    S=width
    ;;
  [0-9] | [0-9][0-9] )
    ADDS="${ADDS} ${A}"
    ;;
  -[0-9] | -[0-9][0-9] )
    ADDS="${ADDS} $( print -- ${A} | cut -c 2-10 )"
    ;;
  * )
    case ${S} in
    adds )
      ADDS="${ADDS} ${A}"
      ;;
    width )
      WIDTH=${A}
      S=${OS}
      ;;
    esac
    ;;
  esac
done
#print -u2 "ADDS=${ADDS}"
#print -u2 "WIDTH=${WIDTH}"

OPTS=
if [[ -n "${WIDTH}" ]] ; then
  OPTS="${OPTS} -w ${WIDTH}"
fi

integer num=0
for A in ${ADDS} ; do
  (( num += ${A} ))
done

  if whence holiday > /dev/null ; then
    if [[ -n "${num}" ]] && (( num > 0 )) ; then
      OPTS="${OPTS} -${num}"
    fi
    holiday -o default ${OPTS}
  fi


