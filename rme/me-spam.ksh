#!/bin/ksh
# ME-SPAM

# Mail Encrypted (ME) [PCS program]


VERSION=0
RNODE=rca
RHOST=mail.rightcore.com
RDIR="~/uuts/"
SERVICE=me
GRADE=M


ADDON_ECE=/home/research/dmorano/add-on
ADDON_URI=/abroad/morano/add-on


PROG_WHICH=/bin/which
PROG_DOMAINNAME=/bin/domainname
PROG_FGREP=/bin/fgrep
PROG_CUT=/bin/cut
PROG_UNAME=/bin/uname
PROG_RM=/bin/rm
PROG_CP=/bin/cp
PROG_DATE=date


getinetdomain() {
  if [ -n "${LOCALDOMAIN}" ] ; then
    for D in ${LOCALDOMAIN} ; do
      echo $D
      break
    done
  else
    if [ -r /etc/resolv.conf ] ; then
      ${PROG_FGREP} domain /etc/resolv.conf | while read J1 D J2 ; do
        echo $D
      done
    else
      ${PROG_DOMAINNAME}
    fi
  fi
}


OS_SYSTEM=$( ${PROG_UNAME} -s )
OS_RELEASE=$( ${PROG_UNAME} -r )
ARCH=$( ${PROG_UNAME} -p )

MACH=$( ${PROG_UNAME} -n | ${PROG_CUT} -d . -f 1 )

case $MACH in

glimore* | hammett* )
  DOMAIN=ece.neu.edu
  ;;

* )
  DOMAIN=$( getinetdomain )
  ;;

esac


case ${MACH}.${DOMAIN} in

*.uri.edu )
  : ${LOCAL:=${ADDON_URI}/local}
  : ${NCMP:=${ADDON_URI}/ncmp}
  : ${PCS:=${ADDON_URI}/pcs}
  ;;

*.coe.neu.edu )
  : ${LOCAL:=${HOME}}
  : ${NCMP:=${HOME}}
  : ${PCS:=${HOME}}
  ;;

*.ece.neu.edu )
  : ${LOCAL:=${ADDON_ECE}/local}
  : ${NCMP:=${ADDON_ECE}/ncmp}
  : ${PCS:=${ADDON_ECE}/pcs}
  ;;

sparc*.ece.neu.edu )
  ;;

esac

: ${PCS:=/usr/add-on/pcs}
: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
export PCS LOCAL NCMP


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

IRIX*:*:mips )
  OFD=IRIX
  ;;

esac



addpath() {
  VARNAME=$1
  shift
  if [ $# -ge 1 -a -d "${1}" ] ; then
    eval AA=\${${VARNAME}}
    echo ${AA} | ${PROG_FGREP} "${1}" > /dev/null
    if [ $? -ne 0 ] ; then
      if [ -z "${AA}" ] ; then
          AA=${1}
      else
        if [ $# -eq 1 ] ; then
          AA=${AA}:${1}
        else
          case "${2}" in
          f* | h* )
            AA=${1}:${AA}
            ;;

          * )
            AA=${AA}:${1}
            ;;

          esac
        fi
      fi
      eval ${VARNAME}=${AA}
      export ${VARNAME}
    fi
  fi
  unset VARNAME
}


# add the package area BIN to the user's PATH

addpath PATH ${NCMP}/bin f
addpath PATH ${LOCAL}/bin f

addpath PATH ${PCS}/bin
addpath PATH ${HOME}/bin

addpath PATH /usr/sbin


addpath PATH /usr/local/bin



addpath LD_LIBRARY_PATH ${NCMP}/lib/${OFD} f
addpath LD_LIBRARY_PATH ${LOCAL}/lib/${OFD} f

addpath LD_LIBRARY_PATH ${PCS}/lib/${OFD}
addpath LD_LIBRARY_PATH ${HOME}/lib/${OFD}


addpath LD_LIBRARY_PATH /usr/local/lib


export PATH LD_LIBRARY_PATH


# continue with the work part of the program !

PROG_UUCP=${NCMP}/bin/uucp
PROG_UUPOLL=${LOCAL}/bin/uupoll
if [[ ! -x ${PROG_UUCP} ]] ; then
  PROG_UUCP=/bin/uucp
  PROG_UUPOLL=
fi



OUTFILE=""
RME_KEYFILE=""

F_DEBUG=false
F_SPAM=1

S=blank
OS=""
for A in "${@}" ; do

#echo "arg=${A}" >&2

  case $A in

  '-D' )
    F_DEBUG=true
    ;;

  '-k' )
    OS=${S}
    S=keyfile
    ;;

  '-h' | '-R' )
    OS=${S}
    S=rnode
    ;;

  '-o' )
    OS=${S}
    S=outfile
    ;;

  '-s' )
    OS=${S}
    S=service
    ;;

  '-?' )
#    echo "got here" >&2
    usage
#    echo "got here 2" >&2
    exit 1
    ;;

  '-'* )
    echo "${P}: unknown option \"${A}\" " >&2
    usage()
    exit 1
    ;;

  * )
    case $S in

    service )
      SERVICE=${A}
      S=${OS}
      ;;

    rnode )
      RNODE=${A}
      S=${OS}
      ;;

    outfile )
      OUTFILE=${A}
      S=${OS}
      ;;

    keyfile )
      RME_KEYFILE=${A}
      S=${OS}
      ;;

    esac
    ;;

  esac

done


P=rme


SPOOLDIR=${PCS}/spool/rme
LOGFILE=${PCS}/log/${P}

if [ ! -d ${PCS}/spool ] ; then
  mkdir ${PCS}/spool 2> /dev/null
fi

if [ ! -d ${SPOOLDIR} ] ; then
  mkdir ${SPOOLDIR} 2> /dev/null
fi


: ${LOGNAME:=$( logname )}

: ${TMPDIR:=/tmp}

JF=$( pcsjobfile -r ${TMPDIR} )

JOBID=$( basename $JF )
LID=$( echo $JOBID | cut -c 1-14 )

cleanup() {
  burn $JF
  rm -f $JF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

logprint() {
  echo "${LID}\t${*}" >> $LOGFILE
}


DATE=$( date '+%y/%m/%d %T' )
logprint "${DATE} ${P}/${VERSION}"

N=
if [ -n "${FULLNAME}" ] ; then
  N=${FULLNAME}
else 
  if [ -n "${NAME}" ] ; then
    N=${NAME}
  fi
fi

if [ -n "${N}" ] ; then
  logprint "${MACH}!${LOGNAME} (${N})"
else
  logprint "${MACH}!${LOGNAME}"
fi


if [ -n "${RME_KEYFILE}" ] ; then
  DES_KEYFILE=${RME_KEYFILE}
else
  DES_KEYFILE=${PCS}/etc/rmailerd/password
fi

export DES_KEYFILE

if [[ $F_SPAM -ne 0 ]] ; then
  cex leviathan spamassassin | gzip -c -9 | ecover -j $JOBID | descrypt -e > $JF
else
  gzip -c -9 | ecover -j $JOBID | descrypt -e > $JF
fi


if [ $F_DEBUG != true ] ; then

  if [ -s $JF ] ; then
    if [ -z "${OUTFILE}" ] ; then
      ${PROG_CP} -p $JF ${PCS}/spool/rme/${JOBID}
      ${PROG_UUCP} -r -g $GRADE -C $JF ${RNODE}!${RDIR}/${JOBID}.${SERVICE}
#      if [[ -x "${PROG_UUPOLL}" ]] && pingstat -n $RHOST ; then
#	${PROG_UUPOLL} $RNODE
#      fi
    else
      ${PROG_CP} -p $JF $OUTFILE
    fi
  fi

fi

DATE=$( date '+%y/%m/%d %T' )
logprint "${DATE} completed"

cleanup



