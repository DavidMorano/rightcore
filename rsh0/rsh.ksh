#!/usr/bin/ksh
# RLOGIN/RSH - Remote Login and Remote Shell


: ${LOCALDOMAIN:=rightcore.com}


if [ -d /usr/sbin ] ; then

  RLOGIN=/bin/rlogin
  RSH=/bin/rsh
 
else

  RLOGIN=/usr/ucb/rlogin
  RSH=/usr/ucb/rsh

fi

WHAT=$( basename ${0} )

if [ $# -lt 1 ] ; then

  echo "${WHAT}: no host specified" >&2
  exit 1

fi

if [[ -n "${LOGNAME}" ]] ; then
  RUSER=${LOGNAME}
else
  RUSER=
fi

OPTS=""
S=host
F_GO=false
F_HOST=false
C=""

A=$1
while [ $F_GO != true -a -n "${A}" ] ; do

  shift
  case "${A}" in

  -l )
    OS=${S}
    S=user
    ;;

  -n )
    if [ $WHAT != rlogin ] ; then OPTS="${OPTS} -n" ; fi
    ;;

  -e )
    OS=${S}
    S=escape
    ;;

  -L | -8 )
    if [ $WHAT = rlogin ] ; then OPTS="${OPTS} ${A}" ; fi
    ;;

  * )
    case $S in

    user )
      RUSER=${A}
      S=${OS}
      ;;

    host )
      H=${A}
      S=command
      F_HOST=true
      ;;

    command )
      C=${A}
      F_GO=true
      ;;

    escape )
      if [ $WHAT == rlogin ] ; then OPTS="${OPTS} -e${A}" ; fi
      ;;

    esac
    ;;

  esac

  A=${1}

done

if [ ${F_HOST} = false ] ; then

  echo "${WHAT}: no host specified" >&2
  exit 1

fi

case $H in

[a-z] )
  H=rc${H}.${LOCALDOMAIN}
  ;;

hocp[a-z] )
  H=${H}.${LOCALDOMAIN}
  ;;

mtgz[0-9][0-9][0-9] )
  H=${H}.${LOCALDOMAIN}
  ;;

mtgzfs[0-9] )
  H=${H}.${LOCALDOMAIN}
  ;;

mthost2 )
  H=mthost2.${LOCALDOMAIN}
  ;;

boogie )
  H=boogie.tempo.bell-labs.com
  ;;

* )
  ;;

esac

case $WHAT in

rlogin )
  WHAT=$RLOGIN
  ;;

rsh | remsh )
  WHAT=$RSH
  ;;

esac

if [[ -n "${RUSER}" ]] ; then
  OPTS="${OPTS} -l ${RUSER}"
fi

exec ${WHAT} ${OPTS} $H $C "$@"



