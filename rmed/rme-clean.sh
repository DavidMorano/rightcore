#!/usr/bin/ksh
# RME-CLEAN


RNODE=uri
RDIR="~/uuts"
SERVICE=rme
GRADE=M


ADDON_ECE=/home/research/dmorano/add-on
ADDON_URI=/marlin/morano/add-on


PROG_WHICH=/bin/which
PROG_DOMAINNAME=/bin/domainname
PROG_FGREP=/bin/fgrep
PROG_CUT=/bin/cut
PROG_CAT=/bin/cat
PROG_UNAME=/bin/uname
PROG_RM=/bin/rm



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


OS_SYSTEM=`${PROG_UNAME} -s`
OS_RELEASE=`${PROG_UNAME} -r`
ARCH=`${PROG_UNAME} -p`

MACH=`${PROG_UNAME} -n | ${PROG_CUT} -d . -f 1 `

case $MACH in

glimore* | hammett* )
  DOMAIN=ece.neu.edu
  ;;

* )
  DOMAIN=`getinetdomain`
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


# add the package area BIN to the user's PATH

echo $PATH | fgrep ${PCS}/bin > /dev/null
if [ $? -ne 0 ] ; then
  PATH=${PATH}:${PCS}/bin
fi

if [ ! -d /usr/sbin ] ; then PATH=/usr/5bin:${PATH} ; fi

echo $LD_LIBRARY_PATH | fgrep ${PCS}/lib/${OFD} > /dev/null
if [ $? -ne 0 ] ; then
  LD_LIBRARY_PATH=${PCS}/lib/${OFD}:${LD_LIBRARY_PATH}
fi



addpath() {
  if [ $# -ge 1 -a -d "${1}" ] ; then

    echo ${PATH} | grep "${1}" > /dev/null
    if [ $? -ne 0 ] ; then
      if [ -z "${PATH}" ] ; then
          PATH=${1}
      else
        if [ $# -eq 1 ] ; then
          PATH=${PATH}:${1}
        else
          case "${2}" in

          f* | h* )
            PATH=${1}:${PATH}
            ;;

          * )
            PATH=${PATH}:${1}
            ;;

          esac
        fi
      fi
      export PATH
    fi
  fi
}


addpath ${PCS}/bin
addpath /bin
addpath ${LOCAL}/bin
addpath ${NCMP}/bin
addpath ${GNU}/bin

export PATH




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
    cleanup
    exit 1
    ;;

  '-'* )
    echo "${P}: unknown option \"${A}\" " >&2
    usage()
    cleanup
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


SPOOLDIR=${PCS}/spool/rme

if [ ! -d ${PCS}/spool ] ; then
  mkdir ${PCS}/spool 2> /dev/null
fi

if [ ! -d ${SPOOLDIR} ] ; then
  mkdir ${SPOOLDIR} 2> /dev/null
fi



TF=${TMPDIR:=/tmp}/rme${RANDOM}${$}

cleanup() {
  rm -f $TF
}

trap 'exit 1 ; cleanup' 1 2 3 15 16 17


if [ -n "${RME_KEYFILE}" ] ; then
  DES_KEYFILE=${RME_KEYFILE}
else
  DES_KEYFILE=${PCS}/etc/rmailerd/password
fi

export DES_KEYFILE



find ${SPOOLDIR} -type f -mtime +3 -exec ${PROG_RM} -f {} \;


cleanup



