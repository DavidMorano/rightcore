#!/bin/ksh
# PCSMAIL


if [ -d /usr/sbin ] ; then

  MACH=`uname -n`

else

  MACH=`hostname`
  PATH=/usr/5bin:${PATH}

fi

case $MACH in

hocp[a-d] | nucleus | logicgate | nitrogen )
  : ${PCS:=/home/gwbb/add-on/pcs}
  : ${TOOLS:=/opt/exptools}
  : ${LOCAL:=/home/gwbb/add-on/local}
  : ${MAILDOMAIN:=ho.lucent.com}
  : ${MAILHOST:=hocpa.${MAILDOMAIN}}
  export MAILDOMAIN MAILHOST
  ;;

ho* )
  : ${MAILDOMAIN:=ho.lucent.com}
  : ${MAILHOST:=hocpa.${MAILDOMAIN}}
  export MAILDOMAIN MAILHOST
 ;;

mt* )
  : ${LOCAL:=/mt/mtgzfs8/hw/add-on/local}
  : ${MAILDOMAIN:=mt.lucent.com}
  : ${MAILHOST:=mtgbcs.${MAILDOMAIN}}
  export MAILDOMAIN MAILHOST
  ;;

esac


: ${PCS:=/usr/add-on/pcs}
: ${TOOLS:=/usr/add-on/exptools}
: ${LOCAL:=/usr/add-on/local}
export LOCAL TOOLS PCS


PN=`basename ${0} `



PATH=${PCS}/bin:${PATH}


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

if [ -z "${LOGNAME}" ] ; then

  if [ -n "${USER}" ] ; then

    LOGNAME=$USER

  else

    LOGNAME=`logname`

  fi

fi

export LOGNAME


if [ -n "${LOGNAME}" ] ; then {

  U=${LOGNAME}

  D=/tmp
  find $D -type f -user $U -mtime +3 -name 'smail*' -exec rm -f {} \;
  find $D -type f -user $U -mtime +3 -name 'forward*' -exec rm -f {} \;
  find $D -type f -user $U -mtime +3 -name 'lfile*' -exec rm -f {} \;

  D=/var/tmp
  find $D -type f -user $U -mtime +3 -name '[a-z]aaa[0-9]*' -exec rm -f {} \;

} > /dev/null 2> /dev/null & fi

if [ -r ${HOME}/lib/pcs/my.names ] ; then

  ln ${HOME}/lib/pcs/my.names ${HOME}/.pcsnames 2> /dev/null

else

  if [ -r ${HOME}/mail.lists/my.names ] ; then

    ln ${HOME}/lib/pcs/my.names ${HOME}/.pcsnames 2> /dev/null

  fi

fi


# continue


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


addpath ${TOOLS}/bin
addpath ${PCS}/bin
addpath /usr/sbin b
addpath /usr/bin f
addpath ${LOCAL}/bin f

export PATH


# find the 'pname' program if we can

PNAME=${PCS}/etc/bin/pname

if [ ! -x $PNAME ] ; then PNAME=${LOCAL}/bin/pname ; fi

if [ ! -x $PNAME ] ; then PNAME=${LOCAL}/etc/bin/pname ; fi

if [ ! -x $PNAME ] ; then PNAME=${TOOLS}/bin/execv ; fi

if [ ! -x $PNAME ] ; then PNAME=`whence pname` ; fi

if [ ! -x "${PNAME}" ] ; then 

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



