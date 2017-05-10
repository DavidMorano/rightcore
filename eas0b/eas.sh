#!/bin/ksh
# XXX


: ${PCS:=/usr/add-on/pcs}
: ${TOOLS:=/usr/add-on/exptools}
: ${LOCAL:=/usr/add-on/local}
: ${PROJTOOLS:=/proj/starbase/tools}
export LOCAL TOOLS PCS PROJTOOLS


if [ -d /usr/sbin ] ; then

  MACH=`uname -n`

else

  MACH=`hostname`
  PATH=/usr/5bin:${PATH}

fi


# check up on PATH stuff

if [ -z "${PATH}" ] ; then PATH=/usr/bin ; fi

echo $PATH | fgrep /usr/bin > /dev/null
if [ $? -ne 0 ] ; then PATH=/usr/bin:${PATH} ; fi


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
    fi
  fi
}


addpath /usr/sbin b
addpath /usr/bin f
addpath ${LOCAL}/bin f
addpath ${PROJTOOLS}/bin f

export PATH


# find the 'pname' program if we can

PNAME=${LOCAL}/etc/bin/pname

if [ ! -x $PNAME ] ; then PNAME=${LOCAL}/bin/pname ; fi

if [ ! -x $PNAME ] ; then PNAME=${PCS}/etc/bin/pname ; fi

if [ ! -x $PNAME ] ; then PNAME=${TOOLS}/bin/execv ; fi

if [ ! -x $PNAME ] ; then PNAME=${PROJTOOLS}/bin/pname ; fi

if [ ! -x $PNAME ] ; then PNAME=`whence pname` ; fi

if [ ! -x $PNAME ] ; then 

  if [ -x ${TOOLS}/bin/where ] ; then

    PNAME=`${TOOLS}/bin/where pname`

  fi

fi


# function to EXEC a program, the best that we can

execprog() {
  PROG_PATH=$1
  shift
  PROG_NAME=$1
  shift
  if [ -x $PNAME ] ; then

    exec $PNAME $PROG_PATH $PROG_NAME "${@}"

  else

    exec $PROG_PATH "${@}"

  fi
}


# get our program name

P=`basename ${0} `


# run the program intended, finally

if [ -d /usr/sbin ] ; then

  if whence ${P}.elf > /dev/null ; then execprog ${P}.elf $P "${@}" ; fi

  if whence ${P}.x > /dev/null ; then execprog ${P}.x $P "${@}" ; fi

  if whence ${P}.aout > /dev/null ; then execprog ${P}.aout $P "${@}" ; fi

else

  if whence ${P}.aout > /dev/null ; then execprog ${P}.aout $P "${@}" ; fi

  if whence ${P}.x > /dev/null ; then execprog ${P}.x $P "${@}" ; fi

fi


echo "${P}: could not find the underlying \"${P}\" program" >&2
exit 1



