#!/usr/bin/ksh
# RMAILER

#echo rmailer2=${LD_LIBRARY_PATH} >&2

# (must run with KSH)

#
# Synopsis:
#
#	rmailer [recepient(s)] [-f from] [-M mailer] [-h host] [-r file]
#
#

#echo "${*}" >&2


# use an alternate mailhost?
F_ALTMAILHOST=0


MAILDOMAIN=ele.uri.edu
RHOST=piranha
MAILBRIDGE_MAILHOST=frodo
MC_TO=3
MC_HOSTS="mail.rightcore.com "

TRANSPORT_HOST=levo.rightcore.com
MSGENV_FROMADDR=morano@ele.uri.edu
RPORT=5107

P=rmailer
VERSION=0


P_PS=/bin/ps
P_WHICH=/bin/which
P_DOMAINNAME=/bin/domainname
P_FGREP=/bin/fgrep
P_CUT=/bin/cut
P_UNAME=/bin/uname
P_BASENAME=/bin/basename
P_DATE=/bin/date
P_RM=/bin/rm
P_MAILER=/usr/bin/rmail
P_SENDMAIL=/usr/postfix/bin/sendmail



getinetdomain() {
  if [ -n "${LOCALDOMAIN}" ] ; then
    for D in ${LOCALDOMAIN} ; do
      echo $D
      break
    done
  else
    if [ -r /etc/resolv.conf ] ; then
      ${P_FGREP} domain /etc/resolv.conf | while read J1 D J2 ; do
        echo $D
      done
    else
      ${P_DOMAINNAME}
    fi
  fi
}

OS_SYSTEM=$( ${P_UNAME} -s )
OS_RELEASE=$( ${P_UNAME} -r )
ARCH=$( ${P_UNAME} -p )

DOMAIN=$( getinetdomain )

MACH=$( ${P_UNAME} -n | ${P_CUT} -d . -f 1 )


case ${MACH}.${DOMAIN} in

*.coe.neu.edu )
  : ${LOCAL:=${HOME}}
  : ${NCMP:=${HOME}}
  : ${PCS:=${HOME}}
  ;;

sparc1.ece.neu.edu )
  ;;

*.ece.neu.edu )
  : ${LOCAL:=${HOME}/add-on/local}
  : ${NCMP:=${HOME}/add-on/ncmp}
  : ${PCS:=${HOME}/add-on/pcs}
  ;;

*.uri.edu )
  : ${LOCAL:=/marlin/wtan/add-on/local}
  : ${NCMP:=/marlin/wtan/add-on/ncmp}
  : ${PCS:=/marlin/wtan/add-on/pcs}
  ;;

esac

: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
export LOCAL NCMP PCS


case ${OS_SYSTEM}:${OS_RELEASE}:${ARCH} in

SunOS:5*:sparc )
  OFD=S5
  ;;

SunOS:4*:sparc )
  OFD=S4
  ;;

OSF*:*:alpha )
  OFD=OSF
  ;;

esac


# add the package area BIN to the user's PATH

FPATH=${PCS}/fbin
if [[ ! -d $FPATH ]] ; then
  FPATH=${LOCAL}/fbin
fi
export FPATH


addpath PATH ${LOCAL}/bin
addpath LD_LIBRARY_PATH ${LOCAL}/lib

addpath PATH ${PCS}/bin
addpath LD_LIBRARY_PATH ${PCS}/lib

export PATH



# should not have to make any modifications beyond this point

: ${TMPDIR:=/tmp}


JF=$( pcsjobfile -r )

TF=${TMPDIR}/rm${RANDOM}${$}

#echo $JF >&2

typeset -L14 JID=$( $P_BASENAME ${JF} )

cleanup() {
  $P_RM -fr $JF $TF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

usage() {
#    echo "got here u" >&2
  echo "${P}: USAGE> rmailer [recepient(s)] [-f from] [-M mailer] " >&2
  echo "\t\t[-h host] [-r recipfile] [-o outfile]" >&2
}


RECIPIENTS=""
FROM=""
RFILE=""
OUTFILE=""

F_DEBUG=0
F_VERSION=false
F_FROM=false
F_MAILER=false

S=recepients
OS=""
for A in "${@}" ; do

#echo "arg=${A}" >&2

  case $A in

  '-D' )
    F_DEBUG=1
    ;;

  '-V' )
    F_VERSION=true
    ;;

  '-M' )
    OS=${S}
    S=mailer
    ;;

  '-f' )
    OS=${S}
    S=from
    ;;

  '-h' )
    OS=${S}
    S=rhost
    ;;

  '-o' )
    OS=${S}
    S=outfile
    ;;

  '-r' )
    OS=${S}
    S=rfile
    ;;

  '-?' )
#    echo "got here" >&2
    usage
#    echo "got here 2" >&2
    cleanup
    exit 1
    ;;

  '-'* )
    echo "${P}: unknown option \"${A}\" " >&2
    usage
    cleanup
    exit 1
    ;;

  * )
    case $S in

    recepients )
      RECIPIENTS="${RECIPIENTS} ${A}"
#      echo $RECIPIENTS >&2
      ;;

    from )
      FROM=${A}
      F_FROM=true
      S=${OS}
      ;;

    mailer )
      P_MAILER=${A}
      F_MAILER=true
      S=${OS}
      ;;

    rhost )
      RHOST=${A}
      S=${OS}
      ;;

    outfile )
      OUTFILE=${A}
      S=${OS}
      ;;

    rfile )
      RFILE=${A}
      S=${OS}
      ;;

    esac
    ;;

  esac

done


if [ $F_VERSION = true ] ; then
  echo "${P}: version ${VERSION}" >&2
  exit 1
fi


OPTS=""

if [ -n "${RFILE}" -a ! -r "${RFILE}" ] ; then
  echo "${P}: could not read recipient file \"${RFILE}\"" >&2
  cleanup
  exit 1
fi

if [ -z "${RECIPIENTS}" -a ! -r "${RFILE}" ] ; then
  echo "${P}: no recepients specified" >&2
  cleanup
  exit 1
fi


LOGFILE=${PCS}/log/${P}

: ${USERNAME:=$( username )}

DATE=$( $P_DATE '+%y%m%d_%H%M:%S_%Z' )

function logger {
  print "${JID}\t${*}" >> $LOGFILE
}

# initial log entry
{

  echo "${JID}\t${DATE} ${P} ${VERSION}" 
  if [ -n "${FULLNAME}" ] ; then
    echo "${JID}\t${MACH}!${USERNAME} (${FULLNAME})"
  else
    echo "${JID}\t${MACH}!${USERNAME}"
  fi

  echo "${JID}\tjob=${JID}"

  echo "${JID}\thost=${RHOST}"

} >> ${LOGFILE}

chmod go+rw ${LOGFILE} 2> /dev/null


RMAIL_OPTS=""

# capture the STDIN
cat > $TF


# write out the MSGID to the log

MSGID=$( ema $TF -h message-id )

echo "${JID}\tmsgid=${MSGID}" >> ${LOGFILE}


# get any recipients from the recipient file
if [[ -s "${RFILE}" ]] ; then
  while read R J ; do
    case $R in
    '' | '#'* )
      ;;
    * )
      RECIPIENTS="${RECIPIENTS} ${R}"
      ;;
    esac
  done < $RFILE
fi


# log all recipients
logger recipients:
for R in $RECIPIENTS ; do
  logger "  ${R}"
done


{
  if [[ -n "${OUTFILE}" ]] ; then
    print ${RMAIL_OPTS} $RECIPIENTS >&2
    cat > ${OUTFILE} < $TF
  else
      if [[ $F_ALTMAILHOST -ne 0 ]] && inetping -n $RHOST edu ; then
        MAILBRIDGE_MAILHOST=${RHOST}
        export MAILBRIDGE_MAILHOST
        mailbridge $RECIPIENTS < $TF
      else
        ${P_SENDMAIL} -oi $RECIPIENTS < $TF
      fi
  fi
}


if [[ $F_DEBUG -ne 0 ]] ; then
  cat $TF > /tmp/rm$( $P_BASENAME ${TF} )
fi


cleanup



