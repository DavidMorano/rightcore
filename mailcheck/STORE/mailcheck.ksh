#!/bin/ksh
# MAILCHECK


ADDON_ECE=/proj/arch1/dmorano/add-on
ADDON_URI=/abroad/morano/add-on


RNODE=rca
SERVICE=me
TO_LOCK=10
TO_REMOVE=10m
RHOST=mail.rightcore.com
RDIAL="udp:5110"

FEMA="dam"
P=forwardmail.cron
PROGRAMROOT=${HOME}
FLAGFILE=${PROGRAMROOT}/etc/fwm



PROG_PS=/bin/ps
PROG_UNAME=/bin/uname
PROG_CUT=/bin/cut
PROG_CAT=/bin/cat
PROG_FGREP=/bin/fgrep
PROG_CP=/bin/cp
PROG_BASENAME=/bin/basename
PROG_DIRNAME=/bin/dirname
PROG_CHMOD=/bin/chmod
PROG_DOMAINNAME=/bin/domainname
PROG_BZIP2=bzip2
PROG_FORMAIL=formail
PROG_GETMAIL=getmail
PROG_RME=rme
PROG_EMA=ema



OS_SYSTEM=$( ${PROG_UNAME} -s )
OS_RELEASE=$( ${PROG_UNAME} -r )
ARCH=$( ${PROG_UNAME} -p )


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

MACH=$( ${PROG_UNAME} -n | ${PROG_CUT} -d '.' -f 1 )

case $MACH in

gilmore | hammett )
  DOMAIN=ece.neu.edu
  ;;

* )
  DOMAIN=$( getinetdomain )
  ;;

esac

export NODE DOMAIN


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

IRIX*:*:mips )
  OFD=IRIX
  ;;

esac

OFF=$( echo $OFD | tr '[A-Z]' '[a-z]' )


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


addpath LD_LIBRARY_PATH ${NCMP}/lib f
addpath LD_LIBRARY_PATH ${NCMP}/lib/${OFD} f
addpath LD_LIBRARY_PATH ${LOCAL}/lib f
addpath LD_LIBRARY_PATH ${LOCAL}/lib/${OFD} f

addpath LD_LIBRARY_PATH ${PCS}/lib/${OFD}
addpath LD_LIBRARY_PATH ${PCS}/lib


keyauth

P=$( basename ${0} )


if [ ${#} -gt 0 ] ; then
  USERS="${*}"
else
  USERS=" morano "
fi

pcsconf -pl $P $P -y "polling> ${USERS}"


: ${MAILDIR:=/var/mail}

F=0
for U in $USERS ; do

  MAIL=${MAILDIR}/${U}
  if [ -s ${MAIL} ] ; then
    
    mailpop -u $FEMA $RHOST -d $RDIAL 2> /dev/null
    if [[ $? -eq 0 ]] ; then
      F=1
    fi

  fi

done

if [[ $F -ne 0 ]] ; then
  pcsuusched &
fi



