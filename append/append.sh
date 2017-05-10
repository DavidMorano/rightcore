#!/bin/ksh
# XXX (some general program)


PROG_WHICH=/bin/which


getinetdomain() {
  if [ -r /etc/resolv.conf ] ; then
    fgrep domain /etc/resolv.conf | read J1 D J2
    echo $D
  else
    domainname
  fi
}

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


# add the package area BIN to the user's PATH

echo $PATH | fgrep ${PCS}/bin > /dev/null
if [ $? -ne 0 ] ; then
  PATH=${PATH}:${PCS}/bin
fi

if [ ! -d /usr/sbin ] ; then PATH=/usr/5bin:${PATH} ; fi


# find the 'execname' program if we can

EXECNAME=${LOCAL}/bin/execname
if [ ! -x $EXECNAME ] ; then EXECNAME=${LOCAL}/etc/bin/execname ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=${PCS}/bin/execname ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=${PCS}/etc/bin/execname ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=${NCMP}/bin/execname ; fi

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


OS_SYSTEM=`uname -s`
OS_RELEASE=`uname -r`
ARCH=`uname -p`

# run the program intended, finally

case ${OS_SYSTEM}:${OS_RELEASE}:${ARCH} in

SunOS:5*:sparc )
  if whence ${P}.s5 > /dev/null ; then
    execprog ${P}.s5 $P "${@}"
  fi

  if whence ${P}.elf > /dev/null ; then
    execprog ${P}.elf $P "${@}"
  fi

  if whence ${P}.x > /dev/null ; then
    execprog ${P}.x $P "${@}"
  fi

  if whence ${P}.s4 > /dev/null ; then
    execprog ${P}.s4 $P "${@}"
  fi

  if whence ${P}.aout > /dev/null ; then
    execprog ${P}..aout $P "${@}"
  fi
  ;;

SunOS:4*:sparc )
  if whence ${P}.s4 > /dev/null ; then
    execprog ${P}.s4 $P "${@}"
  fi

  if whence ${P}.aout > /dev/null ; then
    execprog ${P}.aout $P "${@}"
  fi

  if whence ${P}.x > /dev/null ; then
    execprog ${P}.x $P "${@}"
  fi
  ;;

OSF:*:alpha )
  if whence ${P}.osf > /dev/null ; then
    execprog ${P}.osf $P "${@}"
  fi
  ;;

esac

echo "${P}: could not find underlying program" >&2
exit 1



