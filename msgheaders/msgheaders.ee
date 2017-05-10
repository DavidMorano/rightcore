#!/bin/ksh
# PCSXXX (some PCS program)


ADDON_ECE=/home/research/dmorano/add-on
ADDON_URI=/abroad/morano/add-on


PROG_WHICH=/bin/which
PROG_DOMAINNAME=/bin/domainname
PROG_FGREP=/bin/fgrep
PROG_CUT=/bin/cut
PROG_UNAME=/bin/uname


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
      ${PROG_DOMAINNAME} | ${PROG_CUT} -d . -f 2-10
    fi
  fi
}


OS_SYSTEM=$( ${PROG_UNAME} -s )
OS_RELEASE=$( ${PROG_UNAME} -r )
ARCH=$( ${PROG_UNAME} -p )

if [ -z "${NODE}" ] ; then
  NODE=$( ${PROG_UNAME} -n | ${PROG_CUT} -d . -f 1 )
fi

if [ -z "${DOMAIN}" ]; then

  case $NODE in

  hammett* )
    DOMAIN=ece.neu.edu
    ;;

  * )
    DOMAIN=$( getinetdomain )
    ;;

  esac

fi


case ${NODE}.${DOMAIN} in

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

SubOS:5.[789]*:sparcu )
  OFD=S5U
  ;;

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

addpath PATH ${PCS}/sbin


addpath LD_LIBRARY_PATH ${NCMP}/lib/${OFD} f
addpath LD_LIBRARY_PATH ${LOCAL}/lib/${OFD} f

addpath LD_LIBRARY_PATH ${PCS}/lib/${OFD}
addpath LD_LIBRARY_PATH ${HOME}/lib/${OFD}



# find the 'execname' program if we can

EXECNAME=${LOCAL}/bin/execname
if [ ! -x $EXECNAME ] ; then EXECNAME=${LOCAL}/etc/bin/execname ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=${PCS}/bin/execname ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=${PCS}/etc/bin/execname ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=execname ; fi


# function to EXEC a program, the best that we can

execprog() {
  PROG_PATH=$1
  shift
  PROG_NAME=$1
  shift
  if whence $EXECNAME > /dev/null ; then
    exec $EXECNAME $PROG_PATH $PROG_NAME "${@}"
  else
    exec $PROG_PATH "${@}"
  fi
}


# get our program name

P=$( basename ${0} )



: ${TERMINFO:=${PCS}/lib/terminfo}
export TERMINFO


# run the program intended, finally

case ${OS_SYSTEM}:${OS_RELEASE}:${ARCH} in

SunOS:5*:sparc )
  if [ -x ${PCS}/bin/${P}.s5 ] ; then
    execprog ${P}.s5 $P "${@}"
  fi

  if [ -x ${PCS}/bin/${P}.elf ] ; then
    execprog ${P}.elf $P "${@}"
  fi

  if [ -x ${PCS}/bin/${P}.x ] ; then
    execprog ${P}.x $P "${@}"
  fi

  if [ -x ${PCS}/bin/${P}.s4 ] ; then
    execprog ${P}.s4 $P "${@}"
  fi

  if [ -x ${PCS}/bin/${P}.aout ] ; then
    execprog ${P}.aout $P "${@}"
  fi
  ;;

SunOS:4*:sparc )
  if [ -x ${PCS}/bin/${P}.s4 ] ; then
    execprog ${P}.s4 $P "${@}"
  fi

  if [ -x ${PCS}/bin/${P}.aout ] ; then
    execprog ${P}.aout $P "${@}"
  fi

  if [ -x ${PCS}/bin/${P}.x ] ; then
    execprog ${P}.x $P "${@}"
  fi
  ;;

OSF*:*:alpha )
  if [ -x ${PCS}/bin/${P}.osf ] ; then
    execprog ${P}.osf $P "${@}"
  fi
  ;;

IRIX*:*:mips )
  if [ -x ${PCS}/bin/${P}.irix ] ; then
    execprog ${P}.irix $P "${@}"
  fi
  ;;

esac

if [ -x ${PCS}/bin/${P}.ksh ] ; then
  execprog ${P}.ksh $P "${@}"
fi

if [ -x ${PCS}/bin/${P}.sh ] ; then
  execprog ${P}.sh $P "${@}"
fi


echo "${P}: could not find underlying program" >&2
exit 1



