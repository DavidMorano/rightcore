#!/usr/bin/ksh
# SMAIL


EMAS=

RF_DEBUG=false

S=files
OS=""
for A in "${@}" ; do

  case $A in

  '-D' )
    F_DEBUG=true
    ;;

  '-'* )
    echo "${P}: unknown option \"${A}\"" >&2
    exit 1
    ;;

  * )
    case $S in

    files )
      if [[ -n "${EMAS}" ]] ; then
        EMAS="${EMAS}­${A}"
      else
        EMAS="${A}"
      fi
      ;;

    esac
    ;;

  esac

done


SVC=rmail
SRVFNAME=/tmp/local/tcpmuxd/srv
append /proto/ussmux:${SVC}${SRVFNAME}­${EMAS}



