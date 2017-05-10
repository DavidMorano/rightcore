#!/usr/bin/ksh
# PCSXXX (some PCS program)


ADDON_ECE=/home/student/dmorano/add-on
ADDON_URI=/u/abroad/morano/add-on


P_WHICH=/bin/which
P_DOMAINNAME=/bin/domainname
P_FGREP=/bin/fgrep
P_CUT=/bin/cut
P_TR=/bin/tr
P_UNAME=/bin/uname
P_DIRNAME=/bin/dirname
P_BASENAME=/bin/basename


getinetdomain() {
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


if [[ -z "${NODE}" ]] ; then
  ${P_UNAME} -s -n -r -m -p | {
    read SYSNAME NODENAME RELEASE MACHINE ARCHITECTURE J
  }
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
  ${P_UNAME} -s -r -m -p | read SYSNAME RELEASE MACHINE ARCHITECTURE
fi

case ${SYSNAME}:${RELEASE}:${ARCHITECTURE} in

SunOS:5.*:sparc )
  MACHINE=$( ${P_UNAME} -m )
  case $MACHINE in

  sun4u )
    ARCHTYPE="${ARCHITECTURE}u"
    ;;

  esac
  ;;

esac

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



OFF=$( print $OFD | $P_TR '[A-Z]' '[a-z]' )


addpath() {
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

addpath PATH /bin f
addpath PATH ${NCMP}/bin f
addpath PATH ${AST}/bin f
addpath PATH ${LOCAL}/bin f

addpath PATH ${PCS}/bin
addpath PATH ${HOME}/bin

addpath PATH ${PCS}/sbin


addpath LD_LIBRARY_PATH ${NCMP}/lib/${OFD} f
addpath LD_LIBRARY_PATH ${AST}/lib/${OFD} f
addpath LD_LIBRARY_PATH ${LOCAL}/lib/${OFD} f

addpath LD_LIBRARY_PATH ${PCS}/lib/${OFD}
addpath LD_LIBRARY_PATH ${HOME}/lib/${OFD}



# find the 'execname' program if we can

EXECNAME=${LOCAL}/bin/execname
if [[ ! -x $EXECNAME ]] ; then EXECNAME=${NCMP}/bin/execname ; fi

if [[ ! -x $EXECNAME ]] ; then EXECNAME=${PCS}/bin/execname ; fi

if [[ ! -x $EXECNAME ]] ; then EXECNAME=execname ; fi


# function to EXEC a program, the best that we can

execprog() {
  P_PATH=$1
  shift
  P_NAME=$1
  shift
  if whence $EXECNAME > /dev/null ; then
    exec $EXECNAME $P_PATH $P_NAME "${@}"
  else
    exec $P_PATH "${@}"
  fi
}


# get our program name

PDIR=$( ${P_DIRNAME} ${0} )
A=${0##*/}
PNAME=${A%.*}

#print -u2 pdir=${PDIR} pname=${PNAME}


: ${TERMINFO:=${PCS}/lib/terminfo}
export TERMINFO


# run the program intended, finally

case ${SYSNAME}:${RELEASE}:${ARCHTYPE} in

SunOS:5.[789]*:sparcu )
  if [[ -x ${PCS}/bin/${PNAME}.s5u ]] ; then
    execprog ${PNAME}.s5u $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.us5 ]] ; then
    execprog ${PNAME}.us5 $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.s5 ]] ; then
    execprog ${PNAME}.s5 $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.elf ]] ; then
    execprog ${PNAME}.elf $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.x ]] ; then
    execprog ${PNAME}.x $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.s4 ]] ; then
    execprog ${PNAME}.s4 $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.aout ]] ; then
    execprog ${PNAME}.aout $PNAME "${@}"
  fi
  ;;

SunOS:5*:sparc* )
  if [[ -x ${PCS}/bin/${PNAME}.s5 ]] ; then
    execprog ${PNAME}.s5 $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.elf ]] ; then
    execprog ${PNAME}.elf $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.x ]] ; then
    execprog ${PNAME}.x $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.s4 ]] ; then
    execprog ${PNAME}.s4 $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.aout ]] ; then
    execprog ${PNAME}.aout $PNAME "${@}"
  fi
  ;;

SunOS:4*:sparc )
  if [[ -x ${PCS}/bin/${PNAME}.s4 ]] ; then
    execprog ${PNAME}.s4 $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.aout ]] ; then
    execprog ${PNAME}.aout $PNAME "${@}"
  fi

  if [[ -x ${PCS}/bin/${PNAME}.x ]] ; then
    execprog ${PNAME}.x $PNAME "${@}"
  fi
  ;;

OSF*:*:alpha )
  if [[ -x ${PCS}/bin/${PNAME}.osf ]] ; then
    execprog ${PNAME}.osf $PNAME "${@}"
  fi
  ;;

IRIX*:*:mips )
  if [[ -x ${PCS}/bin/${PNAME}.irix ]] ; then
    execprog ${PNAME}.irix $PNAME "${@}"
  fi
  ;;

esac

if [[ -x ${PCS}/bin/${PNAME}.ksh93 ]] ; then
  if [[ -d "${AST}" ]] && [[ -x "${AST}/bin/ksh" ]] ; then
    execprog ${AST}/bin/ksh ksh93 ${PNAME}.ksh93 "${@}"
  fi
fi

if [[ -x ${PCS}/bin/${PNAME}.ksh ]] ; then
  execprog ${PNAME}.ksh $PNAME "${@}"
fi

if [[ -x ${PCS}/bin/${PNAME}.sh ]] ; then
  execprog ${PNAME}.sh $PNAME "${@}"
fi


print "${PNAME}: could not find underlying program" >&2
exit 1



