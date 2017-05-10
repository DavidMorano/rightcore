#!/usr/bin/ksh
# MAILCHECK


ADDON_ECE=/home/student/dmorano/add-on
ADDON_URI=/u/abroad/morano/add-on


PROG_WHICH=/bin/which
PROG_DOMAINNAME=/bin/domainname
PROG_FGREP=/bin/fgrep
PROG_CUT=/bin/cut
PROG_TR=/bin/tr
PROG_UNAME=/bin/uname
PROG_DIRNAME=/bin/dirname
PROG_BASENAME=/bin/basename


getinetdomain() {
  if [[ -n "${LOCALDOMAIN}" ]] ; then
    for D in ${LOCALDOMAIN} ; do
      print $D
      break
    done
  else
    if [[ -r /etc/resolv.conf ]] ; then
      ${PROG_FGREP} domain /etc/resolv.conf | while read J1 D J2 ; do
        print $D
      done
    else
      ${PROG_DOMAINNAME} | ${PROG_CUT} -d . -f 2-10
    fi
  fi
}


if [[ -z "${NODE}" ]] ; then
  ${PROG_UNAME} -s -n -r -p | read SYSNAME NODENAME RELEASE ARCHITECTURE
  NODE=${NODENAME%%.*}
fi


if [[ -z "${DOMAIN}" ]] ; then

  case $NODE in

  hammett* )
    DOMAIN=ece.neu.edu
    ;;

  * )
    DOMAIN=$( getinetdomain )
    ;;

  esac

fi

export NODE DOMAIN


case ${NODE}.${DOMAIN} in

*.uri.edu )
  : ${LOCAL:=${ADDON_URI}/local}
  : ${AST:=${ADDON_URI}/ast}
  : ${NCMP:=${ADDON_URI}/ncmp}
  : ${PCS:=${ADDON_URI}/pcs}
  ;;

*.coe.neu.edu )
  : ${LOCAL:=${HOME}}
  : ${AST:=${HOME}}
  : ${NCMP:=${HOME}}
  : ${PCS:=${HOME}}
  ;;

*.ece.neu.edu )
  : ${LOCAL:=${ADDON_ECE}/local}
  : ${AST:=${ADDON_ECE}/ast}
  : ${NCMP:=${ADDON_ECE}/ncmp}
  : ${PCS:=${ADDON_ECE}/pcs}
  ;;

sparc*.ece.neu.edu )
  ;;

esac

: ${LOCAL:=/usr/add-on/local}
: ${AST:=/usr/add-on/ast}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}

export LOCAL AST NCMP PCS


if [[ -z "${SYSNAME}" ]] ; then
  ${PROG_UNAME} -s -r -p | read SYSNAME RELEASE ARCHITECTURE
fi

case ${SYSNAME}:${RELEASE}:${ARCHITECTURE} in

SunOS:5.*:sparc )
  MACHINE=$( ${PROG_UNAME} -m )
  case $MACHINE in

  sun4u )
    ARCHITECTURE="${ARCHITECTURE}u"
    ;;

  esac
  ;;

esac

case ${SYSNAME}:${RELEASE}:${ARCHITECTURE} in

SubOS:5.[789]*:sparcu )
  OFD=S5U
  ;;

SunOS:5*:sparc* )
  OFD=S5
  ;;

SunOS:4*:sparc )
  OFD=S4
  ;;

OSF*:*:alpha )
  OFD=OSF
  ;;

IRIX*:*:mips )
  OFD=IRIX
  ;;

esac



OFF=$( print $OFD | $PROG_TR '[A-Z]' '[a-z]' )


pathadd() {
  VARNAME=$1
  shift
  if [[ $# -ge 1 ]] && [[ -d "${1}" ]] ; then
    eval AA=\${${VARNAME}}
    print ${AA} | ${PROG_FGREP} "${1}" > /dev/null
    if [[ $? -ne 0 ]] ; then
      if [[ -z "${AA}" ]] ; then
          AA=${1}
      else
        if [[ $# -eq 1 ]] ; then
          AA=${AA}:${1}
        else
          case "${2}" in
          f* | h* )
            AA=${1}:${AA}
            ;;

          * )
            AA=${AA}:${1}
            ;;

          esac
        fi
      fi
      eval ${VARNAME}=${AA}
      export ${VARNAME}
    fi
  fi
  unset VARNAME AA
}


# add the package area BIN to the user's PATH

pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib

pathadd PATH ${AST}/bin
pathadd LD_LIBRARY_PATH ${AST}/lib

pathadd PATH ${NCMP}/bin
pathadd LD_LIBRARY_PATH ${NCMP}/lib

pathadd PATH ${PCS}/bin
pathadd PATH ${PCS}/sbin
pathadd LD_LIBRARY_PATH ${PCS}/lib

pathadd PATH ${HOME}/bin
pathadd LD_LIBRARY_PATH ${HOME}/lib



# get our program name

PDIR=$( ${PROG_DIRNAME} ${0} )
A=${0##*/}
PNAME=${A%.*}



F_DEBUG=0
F_FETCH=0


S=users
OS=
for A in "${@}" ; do

  case $A in

  '-f' )
    F_FETCH=1
    ;;

  '-D' )
    F_DEBUG=1
    ;;

  '-'* )
    echo "${P}: unknown option \"${A}\" " >&2
    exit 1
    ;;

  * )
    case $S in

    users )
      USERS="${USERS} ${A}"
      ;;

    esac
    ;;

  esac

done




if [[ -z "${USERS}" ]] ; then
  USERS=" morano "
fi

pcsconf -pl $P $P -y "polling> ${USERS}"


: ${MAILDIR:=/var/mail}

    DELIVER_MAILBOX=spool
    export DELIVER_MAILBOX


F=0
for U in $USERS ; do

  UHOME=$( logname $U homedir )
  MAILBOX=${UHOME}/mail/${DELIVER_MAILBOX}

  if [[ $F_FETCH -ne 0 ]] ; then
    fetchmail leviathan
  fi > /dev/null

  if [ -s ${MAILBOX} ] ; then
    
    mailpop -u $FEMA $RHOST -d $RDIAL 2> /dev/null
    if [[ $? -eq 0 ]] ; then
      F=1
    fi

  fi

done




