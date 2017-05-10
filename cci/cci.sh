#!/bin/ksh
# PCSXXX (some PCS program)



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
      ${PROG_DOMAINNAME}
    fi
  fi
}

OS_SYSTEM=`${PROG_UNAME} -s`
OS_RELEASE=`${PROG_UNAME} -r`
ARCH=`${PROG_UNAME} -p`

MACH=`${PROG_UNAME} -n | ${PROG_CUT} -d . -f 1 `

case $MACH in

hammett* )
  DOMAIN=ece.neu.edu
  ;;

* )
  DOMAIN=`getinetdomain`
  ;;

esac


case ${MACH}.${DOMAIN} in

*.coe.neu.edu )
  : ${PCS:=${HOME}}
  : ${LOCAL:=${HOME}}
  : ${NCMP:=${HOME}}
  ;;

*.uri.edu )
  : ${PCS:=/marlin/wtan/add-on/pcs}
  : ${LOCAL:=/marlin/wtan/add-on/local}
  : ${NCMP:=/marlin/wtan/add-on/ncmp}
  ;;

sparc*.ece.neu.edu )
  ;;

*.ece.neu.edu )
  : ${PCS:=${HOME}/pcs}
  : ${LOCAL:=${HOME}}
  : ${NCMP:=${HOME}}
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

P=`basename ${0} `



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

esac

if [ -x ${PCS}/bin/${P}.ksh ] ; then
  execprog ${P}.ksh $P "${@}"
fi

if [ -x ${PCS}/bin/${P}.sh ] ; then
  execprog ${P}.sh $P "${@}"
fi


echo "${P}: could not find underlying program" >&2
exit 1



