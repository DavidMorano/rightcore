#!/usr/extra/bin/ksh
# DICTLOOK


: ${LOCAL:=/usr/add-on/local}
: ${GNU:=/usr/add-on/gnu}
: ${EXTRA:=/usr/extra}
export LOCAL GNU EXTRA

PRS=" ${HOME} ${GNU} ${EXTRA} "

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


PN=dictlook
DN=/dev/null

FILES=

RF_DEBUG=false
RF_UNDO=false

S=files
OS=""
for A in "${@}" ; do
  case ${A} in
  '-D' )
    RF_DEBUG=true
    ;;
  '-u' )
    RF_UNDO=true
    ;;
  * )
    case ${S} in
    files )
      FILES="${FILES} ${A}"
      ;;
    esac
    ;;
  esac
done


OPTS=
if ${RF_UNDO} ; then
  OPTS="-S"
else
  OPTS="-s"
fi

: ${COLUMNS:=80}
export COLUMNS

dict "${@}" | linefold -${COLUMNS}


