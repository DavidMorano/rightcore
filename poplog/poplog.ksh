#!/usr/bin/ksh
# POPLOG


MACH=$( uname -n )



LOGHOST=www.rightcore.com
DIALER=tcpmux:1
SERVICE=poplog
PRIORITY=mail.warning
TAG=${MACH}-forwardmail


FILES=

S=files
OS=""
for A in "${@}" ; do

  case $A in

  '-D' )
    F_DEBUG=true
    ;;

  '-p' )
    OS=${S}
    S=priority
    ;;

  '-t' )
    OS=${S}
    S=tag
    ;;

  '-h' )
    OS=${S}
    S=host
    ;;

  '-d' )
    OS=${S}
    S=dialer
    ;;

  '-' )
    F_INPUT=true
    ;;

  '-'* )
    echo "${P}: unknown option \"${A}\"" >&2
    exit 1
    ;;

  * )
    case $S in

    files )
      FILES="${FILES} ${A}"
      ;;

    priority )
      PRIORITY=${A}
      S=${OS}
      ;;

    tag )
      TAG=${A}
      S=${OS}
      ;;

    host )
      LOGHOST=${A}
      S=${OS}
      ;;

    dialer )
      DIALER=${A}
      S=${OS}
      ;;

    esac
    ;;

  esac

done


: ${LOGNAME:=$( logname )}
export LOGNAME


OPTS=
OPTS="${OPTS} -h host=${MACH}"
OPTS="${OPTS} -h logname=${LOGNAME}"
OPTS="${OPTS} -h priority=${PRIORITY}"
if [ -n "${TAG}" ] ; then
  OPTS="${OPTS} -h tag=${TAG}"
fi


mkmsg $OPTS | connect -d $DIALER $LOGHOST -s $SERVICE



