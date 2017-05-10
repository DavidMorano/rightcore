#!/usr/extra/bin/ksh
# RMCAT

RF_DEBUG=false
RF_ERROR=false
RF_VERSION=false

: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
: ${CATMAN:=/usr/add-on/catman}
export LOCAL EXTRA CATMAN

PRS=" ${LOCAL} ${EXTRA}"

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
  pathadd PATH ${PR}/sbin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done
export PATH LD_LIBRARY_PATH


PN=rmcat
DN=/dev/null

EX=0
VERSION="0"
NAMES=

S=names
OS=""
for A in "${@}" ; do
  case ${A} in
  '-D' )
    RF_DEBUG=true
    ;;
  '-V' )
    RF_VERSION=true
    ;;
  * )
    case ${S} in
    names )
      NAMES="${NAMES} ${A}"
      ;;
    esac
    ;;
  esac
done


if [[ -d "${CATMAN}" ]] ; then
   for N in ${NAMES} ; do
     rm -f ${CATMAN}/cat*/${N}.*
   done
fi


