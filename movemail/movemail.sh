#!/bin/ksh
# MOVEMAIL


: ${POPHOST:=lynx.dac.neu.edu}
export POPHOST


if [ -d /usr/sbin ] ; then

  MACH=`uname -n`

else

  MACH=`hostname`
  PATH=/usr/5bin:${PATH}

fi

case $MACH in

mt* )
  : ${LOCAL:=/mt/mtgzfs8/hw/add-on/local}
  ;;

hocp[a-d] | logicgate | nitrogen | nucleus )
  : ${LOCAL:=/home/gwbb/add-on/local}
  : ${PCS:=/home/gwbb/add-on/pcs}
  : ${TOOLS:=/home/gwbb/add-on/exptools}
  ;;

esac


: ${LOCAL:=/usr/add-on/local}
: ${PCS:=/usr/add-on/pcs}
: ${TOOLS:=/usr/add-on/exptools}
export PCS LOCAL TOOLS



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


addpath ${PCS}/bin b
addpath /usr/sbin b
addpath /usr/bin f
addpath ${LOCAL}/bin f
addpath ${PCS}/etc/bin b

export PATH


# find the 'pname' program if we can

PNAME=${PCS}/etc/bin/pname

if [ ! -x $PNAME ] ; then PNAME=${LOCAL}/bin/pname ; fi

if [ ! -x $PNAME ] ; then PNAME=${LOCAL}/etc/bin/pname ; fi

if [ ! -x $PNAME ] ; then PNAME=${TOOLS}/bin/execv ; fi


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



echo "${P}: could not find the underlying ${P} program" >&2
exit 1



