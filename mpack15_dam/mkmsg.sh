#!/bin/ksh
# PCSXXX (some PCS program)


getinetdomain() {
  if [ -r /etc/resolv.conf ] ; then
    fgrep domain /etc/resolv.conf | read J1 D J2
    echo $D
  else
    domainname
  fi
}

OS_SYSTEM=`uname -s`
OS_RELEASE=`uname -r`
ARCH=`uname -p`

if [ -n "${LOCALDOMAIN}" ] ; then
  DOMAIN=${LOCALDOMAIN}
else
  DOMAIN=`getinetdomain`
fi

MACH=`uname -n | cut -d . -f 1 `


case ${MACH}.${DOMAIN} in

*.coe.neu.edu )
  : ${PCS:=${HOME}}
  : ${LOCAL:=${HOME}}
  : ${NCMP:=${HOME}}
  ;;

[bjld]*.ece.neu.edu )
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


# here is a little old PCS hack to get around an old bug !!
if [ -z "${ED}" ] ; then
  if [ -n "${EDITOR}" ] ; then
    ED=$EDITOR
  else
    ED=vi
  fi
  export ED
fi


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
    execprog ${P}..aout $P "${@}"
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

echo "${P}: could not find underlying program" >&2
exit 1



