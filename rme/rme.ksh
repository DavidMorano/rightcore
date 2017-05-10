#!/usr/bin/ksh
# RME

# Remote Mail Encrypted (RME -- PCS program)


VERSION=0
RNODE=uri
#RHOST=leviathan.ele.uri.edu
RHOST=uri
RDIR="~morano/add-on/pcs/spool/uucppublic/uuts/"
SERVICE=rme
GRADE=M


ADDON_ECE=/home/research/dmorano/add-on
ADDON_URI=/abroad/morano/add-on


P_WHICH=which
P_DOMAINNAME=domainname
P_FGREP=fgrep
P_CUT=cut
P_UNAME=uname
P_RM=rm
P_CP=cp
P_BASENAME=basename
P_DATE=date


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

MACH=$( ${P_UNAME} -n | ${P_CUT} -d . -f 1 )

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
  : ${AST:=${ADDON_URI}/ast}
  : ${NCMP:=${ADDON_URI}/ncmp}
  : ${PCS:=${ADDON_URI}/pcs}
  ;;

*.ece.neu.edu )
  : ${LOCAL:=${ADDON_ECE}/local}
  : ${AST:=${ADDON_ECE}/ast}
  : ${NCMP:=${ADDON_ECE}/ncmp}
  : ${PCS:=${ADDON_ECE}/pcs}
  ;;

esac

: ${LOCAL:=/usr/add-on/local}
: ${AST:=/usr/add-on/ast}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
export LOCAL AST NCMP PCS


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



pathadd() {
  VARNAME=$1
  shift
  if [ $# -ge 1 -a -d "${1}" ] ; then
    eval AA=\${${VARNAME}}
    echo ${AA} | ${P_FGREP} "${1}" > /dev/null
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
  unset VARNAME AA
}


# add the package area BIN to the user's PATH

pathadd PATH ${NCMP}/bin f
pathadd PATH ${AST}/bin f
pathadd PATH ${LOCAL}/bin f

pathadd PATH ${PCS}/bin
pathadd PATH ${HOME}/bin

pathadd PATH /usr/sbin


pathadd LD_LIBRARY_PATH ${NCMP}/lib f
pathadd LD_LIBRARY_PATH ${AST}/lib f
pathadd LD_LIBRARY_PATH ${LOCAL}/lib f

pathadd LD_LIBRARY_PATH ${PCS}/lib
pathadd LD_LIBRARY_PATH ${HOME}/lib

export PATH LD_LIBRARY_PATH


# continue with the work part of the program !

#P_UUCP=${NCMP}/bin/uucp
P_UUCP=/bin/uucp
P_UUPOLL=${LOCAL}/bin/uupoll
if [[ ! -x ${P_UUCP} ]] ; then
  P_UUCP=/bin/uucp
  P_UUPOLL=
fi



OUTFILE=""
RME_KEYFILE=""

F_DEBUG=false

S=blank
OS=""
for A in "${@}" ; do

#echo "arg=${A}" >&2

  case $A in

  '-k' )
    OS=${S}
    S=keyfile
    ;;

  '-D' )
    F_DEBUG=true
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

JOBID=$( $P_BASENAME $JF )
LID=$( print $JOBID | $P_CUT -c 1-14 )

cleanup() {
  burn $JF
  $P_RM -f $JF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

logprint() {
  echo "${LID}\t${*}" >> $LOGFILE
}


DATE=$( $P_DATE '+%y%m%d_%H%M:%S_%Z' )
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

gzip -c -9 | ecover -j $JOBID | descrypt -e > $JF


if [ $F_DEBUG != true ] ; then

  if [ -s $JF ] ; then
    if [ -z "${OUTFILE}" ] ; then
      ${P_CP} -p $JF ${PCS}/spool/rme/${JOBID}
      ${P_UUCP} -r -g $GRADE -C $JF ${RNODE}!${RDIR}/${JOBID}.${SERVICE}
      if [[ -x "${P_UUPOLL}" ]] && pingstat -n $RNODE ; then
	${P_UUPOLL} $RNODE
      fi
    else
      ${P_CP} -p $JF $OUTFILE
    fi
  fi

fi

DATE=$( $P_DATE '+%y%m%d_%H%M:%S_%Z' )
logprint "${DATE} completed"

cleanup



