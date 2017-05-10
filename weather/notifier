#!/usr/extra/bin/ksh
# NOTIFIER

#exec 2> /tmp/notifier.err
#set -x

# program version
PV=0b
LOGSIZE=10m
#RF_DEBUG=true
RF_DEBUG=false

# boiler-plate

PDIR=${0%/*}
A=${0##*/}
PFILE=${A%.*}
PN=${PFILE#-}
DN=/dev/null

P_USERHOME=/usr/preroot/bin/userhome
P_WHICH=/usr/bin/which
P_DOMAINNAME=/usr/bin/domainname
P_FGREP=/usr/bin/fgrep
P_CUT=/usr/bin/cut
P_TR=/usr/bin/tr
P_UNAME=/usr/bin/uname
P_DIRNAME=/usr/bin/dirname
P_BASENAME=/usr/bin/basename

if [[ "${FPATH:0:1}" == ":" ]] ; then
  FPATH=${FPATH:1:200}
fi

: ${HOME:=$( ${P_USERHOME} )}
: ${LOCAL:=/usr/add-on/local}
: ${PCS:=/usr/add-on/pcs}
: ${EXTRA:=/usr/extra}
export HOME LOCAL PCS EXTRA

PRS=" ${HOME} ${LOCAL} ${PCS} ${EXTRA} "

for PR in ${PRS} ; do
  if [[ -d ${PR} ]] ; then
    B=${PR}/fbin
    if [[ -d ${B} ]] ; then
      if [[ -n "${FPATH}" ]] ; then
        FPATH="${FPATH}:${B}"
      else
        FPATH=${B}
      fi
    fi
  fi
done
export FPATH

for PR in ${PRS} ; do
  pathadd PATH ${PR}/bin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done

: ${USERNAME:=$( username )}
export USERNAME

# program-root
PR=${LOCAL}

# get our program name
#
#if [[ -n "${_A0}" ]] ; then
#  PN=${_A0}
#fi

if ${RF_DEBUG} ; then
  print -u2 -- "PN=>${PN}<"
fi

keyauth

TMPUSERDIR=$( mktmpuser ${PN} )

LOGFILE=${PR}/log/${PN}

logfile -c ${LOGFILE} -n ${PN}:${PV} -s ${LOGSIZE} |&

function logprint {
  typeset V="${1}"
  if [[ -n "${V}" ]] ; then
    print -p -- "${V}"
  fi
}

TF=/tmp/notifier${$}

function cleanup {
  rm -f ${TF}
}

trap 'cleanup' 1 2 3 15 16 17

DS=$( date '+%T %Z' )

FAC=
TAR=
FRO=
MSG=

S=fac
OS=""
for A in "${@}" ; do
  case ${A} in
  '-D' )
    RF_DEBUG=true
    ;;
  '-f' )
    OS=${S}
    S=from
    ;;
  '-m')
    OS=${S}
    S=msg
    ;;
  '-'* )
    print -u2 -- "${PN}: unknown option \"${A}\""
    FAC=exit
    ;;
  * )
    case ${S} in
    fac )
      FAC="${A}"
      S=tar
      ;;
    tar )
      TAR="${A}"
      S=dump
      ;;
    dump )
      ;;
    from )
      FRO="${A}"
      S=${OS}
      ;;
    msg )
      S=${OS}
      MSG="${A}"
      ;;
    esac
    ;;
  esac
  if [[ "${FAC}" == "exit" ]] ; then break ; fi
done

case ${FAC} in
wire|skype|textonly)
  MSGFILE=${TMPUSERDIR}/notifier${$}
  touch ${MSGFILE}
  logprint "${FAC} ${DS}"
  TU=${TAR}
  if [[ -n "${TU}" ]] && username -q ${TU} ; then
    logprint "user=${TU}"
    logprint "from=> ${FRO}<"
    TXT_LEAD="¥ ${DS} ${FAC} " 
    if ${RF_DEBUG} ; then
      NOTICE_DEBUGFILE=/tmp/notice.deb
      export NOTICE_DEBUGFILE
      z ${NOTICE_DEBUGFILE}
    fi
    {
      TXT=${TXT_LEAD}
      if [[ -n "${FRO}" ]] ; then
        TXT="${TXT} from=> ${FRO} <"
      fi
      print -- "${TXT}"
      if [[ -n "${MSG}" ]] ; then
        print -- "¥ msg= ${MSG:0:70}"
      fi
    } | tee ${MSGFILE} | notice ${TU} -r -3
    mkmsg < ${MSGFILE} -s "${TXT_LEAD}" ${TU} > ${TF}
    dmailbox -nm -m notices ${TU} < ${TF}
    imail ${TU} < ${TF}
    rm -f ${MSGFILE}
    print -- "notified ${DS}"
  else
    print -- "target recipient does not exist ­ ${DS}"
    logprint "target recipient does not exist ­ ${DS}"
  fi
  ;;
*)
  print -u2 -- "${PN}: no facility"
  logprint "no facility"
  ;;
esac

TXT=
if [[ "${FAC}" == "exit" ]] ; then TXT="exit" ; fi
logprint done ${TXT}

cleanup


