#!/bin/ksh
# POPBULL


#
# Synopsis :
#
#	popbull spooldir
#
#


P=popbull


MACH=`uname -n`

case $MACH in

hocp[a-d] | nucleus | logicgate | nitrogen )
  : ${PCS:=/home/gwbb/add-on/pcs}
  : ${LOCAL:=/home/gwbb/add-on/local}
  : ${NCMP:=/home/gwbb/add-on/ncmp}
  : ${TOOLS:=/opt/exptools}
  ;;

esac

: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${TOOLS:=/usr/add-on/exptools}
: ${PCS:=/usr/add-on/pcs}
export LOCAL NCMP TOOLS PCS



export FPATH=${LOCAL}/fbin


# add the package area BIN to the user's PATH

addpath PATH ${LOCAL}/bin
addpath PATH ${NCMP}/bin
addpath PATH ${TOOLS}/bin

addpath PATH ${PCS}/bin
addpath PATH ${PCS}/sbin

if [ ! -d /usr/sbin ] ; then 
  addpath PATH /usr/5bin
fi


# should not have to make any modifications beyond this point

: ${TMPDIR:=/tmp}



FROM=
MAILER=
SPOOLDIR=/var/spool/popbulls

F_DEBUG=false
F_FROM=false
F_SPOOLDIR=false
F_MAILER=false

S=spooldir
OS=""
for A in "${@}" ; do

  case $A in

  '-f' )
    OS=${S}
    S=from
    ;;

  '-D' )
    F_DEBUG=true
    ;;

  '-M' )
    OS=${S}
    S=mailer
    ;;

  '-'* )
    echo "${P}: unknown option \"${A}\" " >&2
    exit 1
    ;;

  * )
    case $S in

    spooldir )
      SPOOLDIR=${A}
      F_SPOOLDIR=true
      S=${OS}
      ;;

    from )
      FROM=${A}
      F_FROM=true
      S=${OS}
      ;;

    mailer )
      PROG_MAILER=${A}
      F_MAILER=true
      S=${OS}
      ;;

    esac
    ;;

  esac

done



# make a log entry
pcsconf -p $P -l $P


OPTS=

if [ -z "${SPOOLDIR}" ] ; then
  SPOOLDIR=/var/spool/popbulls
fi


if [ "${F_DEBUG}" = true ] ; then
  echo "${P}: spooldir=${SPOOLDIR}" >&2
fi


if [ ${F_FROM} = true ] ; then
  OPTS="${OPTS} -f ${FROM}"
fi

if [ $F_MAILER = true ] ; then
  OPTS="${OPTS} -M ${PROG_MAILER}"
fi


MSGS_SPOOLDIR=${SPOOLDIR}
export MSGS_SPOOLDIR

msgs -s

exit 0


