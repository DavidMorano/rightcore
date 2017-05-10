#!/usr/bin/ksh
# MKUUNAMES


#
# Synopsis:
#
# $ mkuunames [-ROOT <pr>]
#
#

#/usr/bin/env > ${HOME}/rje/mkuunames.env


ADDON_ECE=${HOME}/add-on
ADDON_URI=${HOME}/add-on


P_WHICH=/usr/bin/which
P_DOMAINNAME=/usr/bin/domainname
P_FGREP=/usr/bin/fgrep
P_CUT=/usr/bin/cut
P_TR=/usr/bin/tr
P_UNAME=/usr/bin/uname
P_DIRNAME=/usr/bin/dirname
P_BASENAME=/usr/bin/basename


function getinetdomain {
  if [[ -n "${LOCALDOMAIN}" ]] ; then
    for D in ${LOCALDOMAIN} ; do
      print $D
      break
    done
  else
    if [[ -r /etc/resolv.conf ]] ; then
      ${P_FGREP} domain /etc/resolv.conf | while read J1 D J2 ; do
        print $D
      done
    else
      ${P_DOMAINNAME} | ${P_CUT} -d . -f 2-10
    fi
  fi
}


F=0
for N in SYSNAME NODE RELEASE MACHINE ARCHITECTURE ; do
  V=$( eval print \${${N}} )
  if [[ -z "${V}" ]] ; then
    F=1
    break
  fi
done

if [[ $F -ne 0 ]] ; then
  ${P_UNAME} -s -n -r -m -p | {
    read SYSNAME NODENAME RELEASE MACHINE ARCHITECTURE J
  }
  export SYSNAME RELEASE MACHINE ARCHITECTURE
  NODE=${NODENAME%%.*}
  export NODE
fi

#print -u2 SYSNAME=${SYSNAME}
#print -u2 RELEASE=${RELEASE}
#print -u2 MACHINE=${MACHINE}
#print -u2 ARCHITECTURE=${ARCHITECTURE}
#print -u2 NODE=${NODE}

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

export DOMAIN

#print -u2 DOMAIN=${DOMAIN}

case ${NODE}.${DOMAIN} in

*.uri.edu )
  : ${LOCAL:=${ADDON_URI}/local}
  : ${AST:=${ADDON_URI}/ast}
  : ${NCMP:=${ADDON_URI}/ncmp}
  ;;

*.ece.neu.edu )
  : ${LOCAL:=${ADDON_ECE}/local}
  : ${AST:=${ADDON_ECE}/ast}
  : ${NCMP:=${ADDON_ECE}/ncmp}
  ;;

esac

: ${LOCAL:=/usr/add-on/local}
: ${AST:=/usr/add-on/ast}
: ${NCMP:=/usr/add-on/ncmp}

function okexport {
  for A in ${*} ; do
    if [[ -d "${A}" ]] ; then
      export $1
    fi
  done
}

okexport LOCAL AST NCMP


ARCHTYPE=${ARCHITECTURE}
case ${SYSNAME}:${RELEASE}:${ARCHITECTURE} in

SunOS:5.*:sparc )
  case $MACHINE in

  sun4u )
    ARCHTYPE="${ARCHITECTURE}u"
    ;;

  esac
  ;;

esac

#print -u2 ARCHTYPE=${ARCHTYPE}

case ${SYSNAME}:${RELEASE}:${ARCHTYPE} in

SunOS:5.[789]*:sparcu )
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

OFF=$( print $OFD | ${P_TR} '[A-Z]' '[a-z]' )


pathadd() {
  typeset VARNAME AA

  VARNAME=$1
  shift
  if [[ $# -ge 1 ]] && [[ -d "${1}" ]] ; then
    eval AA=\${${VARNAME}}
    print ${AA} | ${P_FGREP} "${1}" > /dev/null
    if [[ $? -ne 0 ]] ; then
      if [[ -z "${AA}" ]] ; then
          AA=${1}
      else
        if [[ $# -eq 1 ]] ; then
          AA=${AA}:${1}
        else
          case "${2}" in
          f* | h* | -f* | -h* )
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


# get our program name

PDIR=${0%/*}
A=${0##*/}
PFILE=${A%.*}
PNAME=${A%.*}
if [[ -n "${_A0}" ]] ; then
  PNAME=${_A0}
fi



pathadd PATH ${NCMP}/bin -f
pathadd LD_LIBRARY_PATH ${NCMP}/lib -f

pathadd PATH ${AST}/bin -f
pathadd LD_LIBRARY_PATH ${AST}/lib -f

pathadd PATH ${LOCAL}/bin -f
pathadd LD_LIBRARY_PATH ${LOCAL}/lib -f

pathadd PATH ${HOME}/bin

if [[ -d "${PDIR}" ]] ; then
  pathadd PATH ${PDIR}
fi

pathadd LD_LIBRARY_PATH ${HOME}/lib



PN=mkuunames

UUDNAME=uunames
UUDBEXT=sl


FPATH=${LOCAL}/fbin
if [[ ! -d ${FPATH} ]] ; then
  FPATH=${NCMP}/fbin
fi
if [[ ! -d ${FPATH} ]] ; then
  FPATH=${PCS}/fbin
fi
export FPATH


pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib

pathadd PATH ${NCMP}/bin
pathadd LD_LIBRARY_PATH ${NCMP}/lib

pathadd PATH ${PCS}/bin
pathadd LD_LIBRARY_PATH ${PCS}/lib

pathadd PATH /usr/bin
pathadd LD_LIBRARY_PATH /usr/lib


PR=${NCMP}


if [[ -n "${DELIVER_MAILBOX}" ]] ; then
  MAILBOX=${DELIVER_MAILBOX}
else
  MAILBOX=new
fi


USERS=
FROM=
MAILER=
PROTOCOL=

RF_DEBUG=false
RF_FROM=false
RF_MAILBOX=false
RF_MAILER=false

S=users
OS=
for A in "${@}" ; do

  case $A in

  '-f' )
    OS=${S}
    S=from
    ;;

  '-m' )
    OS=${S}
    S=mailbox
    ;;

  '-p' )
    OS=${S}
    S=protocol
    ;;

  '-D' )
    RF_DEBUG=true
    ;;

  '-M' )
    OS=${S}
    S=mailer
    ;;

  '-'* )
    print -u2 "${PNAME}: unknown option \"${A}\" " 
    exit 1
    ;;

  * )
    case $S in

    users )
      USERS="${USERS} ${A}"
      ;;

    from )
      FROM=${A}
      RF_FROM=true
      S=${OS}
      ;;

    mailbox )
      MAILBOX=${A}
      RF_MAILBOX=true
      S=${OS}
      ;;

    mailer )
      MAILER=${A}
      RF_MAILER=true
      S=${OS}
      ;;

    protocol )
      PROTOCOL=${A}
      S=${OS}
      ;;

    esac
    ;;

  esac

done


OPTS=


TD=${PR}/var/${UUDNAME}

if [[ ! -d "${TD}" ]] ; then
  mkdir -p ${TD} 2> /dev/null
fi

UF=${TD}/uunames.${UUDBEXT}
TF=${TD}/nu${$}.${UUDBEXT}n
if [[ -n "${MKUUNAMES_DBNAME}" ]] ; then
  TD=$( dirname ${MKUUNAMES_DBNAME} )
  UF=${MKUUNAMES_DBNAME}.${UUDBEXT}
  TF=${MKUUNAMES_DBNAME}.${UUDBEXT}n
fi

if [[ ! -d "${TD}" ]] ; then
  mkdir -p ${TD} 2> /dev/null
fi


function cleanup {
  rm -f $TF
}

trap 'cleanup' 1 2 3 15 16 17


if whence uuname > /dev/null ; then 
  print UUNAMES
  uuname
fi > $TF

if [[ -s "${TF}" ]] ; then
  mv $TF $UF
fi

cleanup
exit 0



