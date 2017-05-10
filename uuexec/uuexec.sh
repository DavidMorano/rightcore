#!/bin/ksh
# UUEXEC


UUHOME=/home/uucp
LOGFILE=${UUHOME}/log/uuexec


: ${PCS:=/usr/add-on/pcs}
: ${TOOLS:=/usr/add-on/exptools}
: ${LOCAL:=/usr/add-on/local}
export LOCAL TOOLS PCS


if [ -d /usr/sbin ] ; then

  MACH=`uname -n`

else

  MACH=`hostname`
  PATH=/usr/5bin:${PATH}

fi


# check up on PATH stuff

if [ -z "${PATH}" ] ; then PATH=/usr/bin ; fi


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


# add to back of PATH

addpath ${UUHOME}/bin
addpath /usr/sbin b
addpath ${TOOLS}/bin b
addpath ${PCS}/bin b

# add to front of PATH

addpath /usr/bin f
addpath ${LOCAL}/bin f

export PATH


# get our program name

P=`basename ${0} `


# process the user identification stuff

if [ -z "${UU_MACHINE}" ] ; then

  UU_MACHINE=${MACH}

fi

if [ -z "${UU_USER}" ] ; then

  UU_USER=adm

fi

UU_USERNAME=`echo ${UU_USER} | cut -d'!' -f2`

if [ -z "${UU_USERNAME}" ] ; then UU_USERNAME=${UU_USER} ; fi


ERRFILE=/tmp/ef${$}
OUTFILE=/tmp/of${$}

cleanup() {
  rm -f $ERRFILE $OUTFILE
}


PROG=$1
shift


A=`dirname $LOGFILE `
if [ -n "${A}" -a ! -d "${A}" ] ; then mkdir $A ; fi


DATE=`date '+%y/%m/%d %T' `
echo "${DATE} started m=${UU_MACHINE} u=${UU_USERNAME} p=${PROG}" >> $LOGFILE


F_EXEC=false
RS=255
if [ -z "${PROG}" ] ; then

  echo "${P}: no program given to execute" >> $ERRFILE

else

if fgrep $PROG ${UUHOME}/etc/uuexec/cmds > /dev/null ; then

if whence $PROG > /dev/null ; then

  F_EXEC=true
  $PROG "${@}" > /dev/null 2>> $ERRFILE
  RS=$?

else

  echo "${P}: program \"${PROG}\" was not found" >> $ERRFILE

fi

else

  echo "${P}: program \"${PROG}\" is not allowed" >> $ERRFILE

fi

fi

if [ -s $ERRFILE ] ; then

  FROM="${MACH}!uucp"
  SUBJECT="UUEXEC execution errors"
  mailx -r "${FROM}" -s "${SUBJECT}" ${UU_MACHINE}!${UU_USERNAME} < $ERRFILE

fi

O="E"
if [ $F_EXEC = true ] ; then O="RS=${RS}" ; fi

DATE=`date '+%y/%m/%d %T' `
echo "${DATE} ended m=${UU_MACHINE} u=${UU_USERNAME} p=${PROG} ${O}" >> $LOGFILE

cleanup


