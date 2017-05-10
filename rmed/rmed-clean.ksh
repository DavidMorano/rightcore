#!/usr/bin/ksh
# RMED (PCS program)


PROG_PS=/bin/ps
PROG_UNAME=/bin/uname
PROG_CUT=/bin/cut
PROG_FGREP=/bin/fgrep
PROG_DOMAINNAME=/bin/domainname
PROG_RM=/bin/rm


OS_SYSTEM=`${PROG_UNAME} -s`
OS_RELEASE=`${PROG_UNAME} -r `
ARCH=`${PROG_UNAME} -p`


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

MACH=`${PROG_UNAME} -n | ${PROG_CUT} -d '.' -f 1`

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
  : ${LOCAL:=/marlin/wtan/add-on/pcs}
  : ${NCMP:=/marlin/wtan/add-on/pcs}
  ;;

sparc1.ece.neu.edu )
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
: ${GNU:=/usr/add-on/gnu}
export PCS LOCAL NCMP GNU


pathadd() {
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

      export PATH

    fi
  fi
}


pathadd ${PCS}/bin
pathadd /bin
pathadd ${LOCAL}/bin
pathadd ${NCMP}/bin
pathadd ${GNU}/bin

export PATH



TF=${TMPDIR:=/tmp}/rmed${RANDOM}${$}

cleanup() {
  rm -f $TF
}

trap 'exit 1 ; cleanup' 1 2 3 15 16 17

DES_KEYFILE=${PCS}/etc/rmailerd/password
export DES_KEYFILE


SPOOLDIR=${PCS}/spool/rmed


if [ -d ${SPOOLDIR} ] ; then
  find ${SPOOLDIR} -type f -mtime +3 -exec ${PROG_RM} -f {} \;
fi



cleanup


