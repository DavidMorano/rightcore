#!/usr/extra/bin/ksh
# BACKDISK

RF_DEBUG=false
RF_ERROR=false
RF_VERSION=false

# defaults
DISK_SRC=/sb07
DISK_DST=/data07

: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export LOCAL EXTRA

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


PN=backdisk
DN=/dev/null
CONF=${EXTRA}/etc/${PN}.conf

EX=0
VERSION="0"
QUERY=

S=query
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
    query )
      QUERY="${QUERY} ${A}"
      ;;
    esac
    ;;
  esac
done

  if ${RF_DEBUG} ; then
    print -u2 -- "${PN}: conf=${CONF}"
  fi

if [[ ${RF_ERROR} -eq 0 ]] && [[ -r ${CONF} ]] ; then
  if ${RF_DEBUG} ; then
    print -u2 -- "${PN}: processing configuration"
  fi
  while read KEY VAL ; do
    if ${RF_DEBUG} ; then
      print -u2 -- "${PN}: key=${KEY} val=${VAL}"
    fi
    case "${KEY}" in
    '#')
      ;;
    src)
      DISK_SRC=${VAL}
      ;;
    dst)
      DISK_DST=${VAL}
      ;;
    esac
  done < ${CONF}
fi

  if ${RF_DEBUG} ; then
    print -u2 -- "${PN}: src=${DISK_SRC}"
    print -u2 -- "${PN}: dst=${DISK_DST}"
  fi

if [[ ${RF_ERROR} -eq 0 ]] ; then
for Q in ${QUERY} ; do
  case ${Q} in
  src)
    print -- ${DISK_SRC}
    ;;
  dst)
    print -- ${DISK_DST}
    ;;
  *)
    RF_ERROR=true
    ;;
  esac
  if ${RF_ERROR} ; then break ; fi
done
fi

if ${RF_ERROR} ; then
  EX=1
  print -u2 -- "${PN}: error in query"
fi

return ${EX}


