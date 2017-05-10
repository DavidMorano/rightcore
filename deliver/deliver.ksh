#!/usr/bin/ksh
# DELIVER

#
# Synopsis:
# deliver [<user(s)>] [-f <from>] [-m <mailbox>] [-M <mailer>]
#
#


P=deliver


if [[ -z "${NODE}" ]] ; then
  NODE=$( uname -n )
fi

case $NODE in

hocp[a-d] | nucleus | logicgate | nitrogen )
  : ${LOCAL:=/home/gwbb/add-on/local}
  : ${NCMP:=/home/gwbb/add-on/ncmp}
  : ${PCS:=/home/gwbb/add-on/pcs}
  ;;

esac

: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
export LOCAL NCMP PCS


FPATH=${LOCAL}/fbin
if [[ ! -d ${FPATH} ]] ; then
  FPATH=${PCS}/fbin
fi
export FPATH


pathadd PATH ${PCS}/bin
pathadd LD_LIBRARY_PATH ${PCS}/lib

pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib



if [[ -n "${DELIVER_MAILBOX}" ]] ; then
  MAILBOX=${DELIVER_MAILBOX}
else
  MAILBOX=new
fi


USERS=
FROM=
MAILER=
PROTOCOL=

RF_DEBUG=false
RF_FROM=false
RF_MAILBOX=false
RF_MAILER=false

S=users
OS=
for A in "${@}" ; do
  case ${A} in
  '-f' )
    OS=${S}
    S=from
    ;;
  '-m' )
    OS=${S}
    S=mailbox
    ;;
  '-p' )
    OS=${S}
    S=protocol
    ;;
  '-D' )
    RF_DEBUG=true
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
    case ${S} in
    users )
      USERS="${USERS} ${A}"
      ;;
    from )
      FROM=${A}
      RF_FROM=true
      S=${OS}
      ;;
    mailbox )
      MAILBOX=${A}
      RF_MAILBOX=true
      S=${OS}
      ;;
    mailer )
      MAILER=${A}
      RF_MAILER=true
      S=${OS}
      ;;
    protocol )
      PROTOCOL=${A}
      S=${OS}
      ;;
    esac
    ;;
  esac
done


OPTS=

if [[ -n ${MAILBOX} ]] ; then
  OPTS="${OPTS} -m ${MAILBOX}"
fi

if ${RF_FROM} ; then
  OPTS="${OPTS} -f ${FROM}"
fi

#if $RF_MAILER ; then
#  OPTS="${OPTS} -M ${MAILER}"
#fi

if [[ -n "${PROTOCOL}" ]] ; then
  OPTS="${OPTS} -p=${PROTOCOL}"
fi


dmailbox ${OPTS}


