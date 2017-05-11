#!/bin/ksh
# PROGRAM


: ${LOCAL:=/usr/add-on/local}
: ${TOOLS:=/usr/add-on/exptools}
: ${DWBHOME:=/usr/add-on/dwb}
: ${PROJ:=/proj/starbase}
export LOCAL TOOLS DWBHOME PROJ


if [ -d /usr/sbin ] ; then

  MACH=`uname -n`

else

  MACH=`hostname`
  if [ -d /usr/5bin ] ; then PATH=/usr/5bin:${PATH} ; fi

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


addpath /usr/bin f
addpath /usr/5bin f
addpath ${LOCAL}/bin f

addpath /usr/sbin b
addpath ${DWBHOME}/bin
addpath ${PROJ}/tools/bin

export PATH


# find the 'execname' program if we can

PNAME=${LOCAL}/etc/bin/execname

if [ ! -x $PNAME ] ; then PNAME=${LOCAL}/bin/execname ; fi

if [ ! -x $PNAME ] ; then PNAME=${TOOLS}/bin/execv ; fi

if [ ! -x $PNAME ] ; then PNAME=/proj/starbase/tools/bin/execv ; fi

if [ ! -x $PNAME ] ; then PNAME=`whence execname` ; fi

if [ ! -x $PNAME ] ; then 

  if [ -x ${TOOLS}/bin/where ] ; then

    PNAME=`${TOOLS}/bin/where execname`

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


echo "${0}: could not find the underlying \"${P}\" program" >&2
exit 1



