#!/bin/ksh
# UUSM-T

set -x
exec 2>> /tmp/uusm.err



: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${TOOLS:=/usr/add-on/exptools}
: ${PCS:=/usr/add-on/pcs}
export LOCAL NCMP TOOLS PCS


MACH=`uname -n`


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
addpath ${NCMP}/bin b
addpath ${TOOLS}/bin b
addpath ${PCS}/bin b

# add to front of PATH

addpath /usr/bin f
addpath ${LOCAL}/bin f

export PATH


UUHOME=${HOME:=/home/uucp}
LOGFILE=${UUHOME}/log/uuexec

# get our program name

P=`basename ${0} `


# process the user identification stuff

if [ -z "${UU_MACHINE}" ] ; then
  UU_MACHINE=${MACH}
fi

if [ -z "${UU_USER}" ] ; then
  UU_USER=adm
fi

if echo ${UU_USER} | fgrep '!' > /dev/null ; then
  UU_USERNAME=`echo ${UU_USER} | cut -d '!' -f 2`
else
  UU_USERNAME=${UU_USER}
fi

if [ -z "${UU_USERNAME}" ] ; then
  UU_USERNAME=${UU_USER}
fi


ENVFROM=${MACH}!${UU_MACHINE}!${UU_USER}

/usr/lib/sendmail -o i -f ${ENVFROM} -t "${@}"



