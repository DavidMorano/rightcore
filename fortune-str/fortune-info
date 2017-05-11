#!/bin/ksh
# XXX


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
  : ${PCS:=/home/gwbb/add-on/pcs}
  : ${TOOLS:=/opt/exptools}
  : ${LOCAL:=/home/gwbb/add-on/local}
  ;;

esac


: ${PCS:=/usr/add-on/pcs}
: ${TOOLS:=/usr/add-on/exptools}
: ${LOCAL:=/usr/add-on/local}
export LOCAL TOOLS PCS



# make your decision here about who's program this is !

: ${PROGRAMROOT:=${LOCAL}}
export PROGRAMROOT



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
addpath ${PCS}/bin f
addpath ${TOOLS}/bin f
addpath ${LOCAL}/bin f

export PATH


# find the 'execname' program if we can

EXECNAME=${LOCAL}/bin/execname

if [ ! -x $EXECNAME ] ; then EXECNAME=${LOCAL}/etc/bin/execname ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=${PCS}/etc/bin/execname ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=${TOOLS}/bin/execv ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=`whence execname` ; fi

if [ ! -x $EXECNAME ] ; then 

  if [ -x ${TOOLS}/bin/where ] ; then

    EXECNAME=`${TOOLS}/bin/where execname`

  fi

fi

if [ ! -x $EXECNAME ] ; then EXECNAME=${HOME}/bin/execname ; fi


# function to EXEC a program, the best that we can

execprog() {
  PROG_PATH=$1
  shift
  PROG_NAME=$1
  shift
  if [ -x $EXECNAME ] ; then

    exec $EXECNAME $PROG_PATH $PROG_NAME "${@}"

  else

    exec $PROG_PATH "${@}"

  fi
}


# get our program name

P=`basename ${0} `

OS=`uname -s`
ISA=`uname -p`


# run the program intended, finally

case ${OS}:${ISA} in

SunOS:sparc )

if [ -d /usr/sbin ] ; then

  if whence ${P}.elf > /dev/null ; then execprog ${P}.elf $P "${@}" ; fi

  if whence ${P}.x > /dev/null ; then execprog ${P}.x $P "${@}" ; fi

  if whence ${P}.aout > /dev/null ; then execprog ${P}.aout $P "${@}" ; fi

else

  if whence ${P}.aout > /dev/null ; then execprog ${P}.aout $P "${@}" ; fi

  if whence ${P}.x > /dev/null ; then execprog ${P}.x $P "${@}" ; fi

fi
  ;;

OSF*:alpha )
  if whence ${P}.osf > /dev/null ; then 
    execprog ${P}.osf $P "${@}"
  fi
  ;;

esac


echo "${P}: could not find the underlying \"${P}\" program" >&2
exit 1


