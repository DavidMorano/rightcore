#!/usr/bin/ksh
# XXX (some general program)


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

OFF=$( print ${OFD} | ${P_TR} '[A-Z]' '[a-z]' )


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
PBASE=${A%.*}
if [[ -n "${_A0}" ]] ; then
  PNAME=${_A0}
else
  PNAME=${A%.*}
fi


# add the package area BIN to the users PATH

pathadd PATH ${NCMP}/bin -f
pathadd PATH ${AST}/bin -f
pathadd PATH ${LOCAL}/bin -f

pathadd PATH ${HOME}/bin

if [[ -d "${PDIR}" ]] ; then
  pathadd PATH ${PDIR}
fi


pathadd LD_LIBRARY_PATH ${NCMP}/lib -f
pathadd LD_LIBRARY_PATH ${NCMP}/lib/${OFD} -f
pathadd LD_LIBRARY_PATH ${AST}/lib -f
pathadd LD_LIBRARY_PATH ${LOCAL}/lib -f
pathadd LD_LIBRARY_PATH ${LOCAL}/lib/${OFD} -f

pathadd LD_LIBRARY_PATH ${HOME}/lib/${OFD}
pathadd LD_LIBRARY_PATH ${HOME}/lib


# find the P_EXECNAME program if we can

P_EXECNAME=${HOME}/bin/execname
if [[ ! -x $P_EXECNAME ]] ; then P_EXECNAME=${LOCAL}/bin/execname ; fi

if [[ ! -x $P_EXECNAME ]] ; then P_EXECNAME=${NCMP}/bin/execname ; fi

if [[ ! -x $P_EXECNAME ]] ; then P_EXECNAME=execname ; fi

if whence $P_EXECNAME > /dev/null ; then :
else
  print -u2 "${0}: software installation error"
  exit 1
fi


# function to EXEC a program, the best that we can

execprog() {
  EXECP_PATH=$1
  shift
  EXECP_NAME=$1
  shift
  if whence $P_EXECNAME > /dev/null ; then
    exec $P_EXECNAME $EXECP_PATH $EXECP_NAME "${@}"
  else
    exec $EXECP_PATH "${@}"
  fi
}



# run the program intended, finally

case ${SYSNAME}:${RELEASE}:${ARCHTYPE} in

SunOS:5.[789]*:sparcu )
  if whence ${PBASE}.s5u > /dev/null ; then
    execprog ${PBASE}.s5u ${PNAME} "${@}"
  fi

  if whence ${PBASE}.us5 > /dev/null ; then
    execprog ${PBASE}.us5 ${PNAME} "${@}"
  fi

  if whence ${PBASE}.s5 > /dev/null ; then
    execprog ${PBASE}.s5 ${PNAME} "${@}"
  fi

  if whence ${PBASE}.elf > /dev/null ; then
    execprog ${PNAME}.elf ${PNAME} "${@}"
  fi

  if whence ${PBASE}.x > /dev/null ; then
    execprog ${PBASE}.x ${PNAME} "${@}"
  fi

  if whence ${PBASE}.s4 > /dev/null ; then
    execprog ${PBASE}.s4 ${PNAME} "${@}"
  fi

  if whence ${PBASE}.aout > /dev/null ; then
    execprog ${PBASE}.aout ${PNAME} "${@}"
  fi
  ;;

SunOS:5*:sparcu )
  if whence ${PBASE}.us5 > /dev/null ; then
    execprog ${PBASE}.us5 ${PNAME} "${@}"
  fi

  if whence ${PBASE}.s5 > /dev/null ; then
    execprog ${PBASE}.s5 ${PNAME} "${@}"
  fi

  if whence ${PBASE}.elf > /dev/null ; then
    execprog ${PBASE}.elf ${PNAME} "${@}"
  fi

  if whence ${PBASE}.x > /dev/null ; then
    execprog ${PBASE}.x ${PNAME} "${@}"
  fi

  if whence ${PBASE}.s4 > /dev/null ; then
    execprog ${PBASE}.s4 ${PNAME} "${@}"
  fi

  if whence ${PBASE}.aout > /dev/null ; then
    execprog ${PBASE}.aout ${PNAME} "${@}"
  fi
  ;;

SunOS:5*:sparc* )
  if whence ${PBASE}.s5 > /dev/null ; then
    execprog ${PBASE}.s5 ${PNAME} "${@}"
  fi

  if whence ${PBASE}.elf > /dev/null ; then
    execprog ${PBASE}.elf ${PNAME} "${@}"
  fi

  if whence ${PBASE}.x > /dev/null ; then
    execprog ${PBASE}.x ${PNAME} "${@}"
  fi

  if whence ${PBASE}.s4 > /dev/null ; then
    execprog ${PBASE}.s4 ${PNAME} "${@}"
  fi

  if whence ${PBASE}.aout > /dev/null ; then
    execprog ${PBASE}.aout ${PNAME} "${@}"
  fi
  ;;

SunOS:4*:sparc )
  if whence ${PBASE}.s4 > /dev/null ; then
    execprog ${PBASE}.s4 ${PNAME} "${@}"
  fi

  if whence ${PBASE}.aout > /dev/null ; then
    execprog ${PBASE}.aout ${PNAME} "${@}"
  fi

  if whence ${PBASE}.x > /dev/null ; then
    execprog ${PBASE}.x ${PNAME} "${@}"
  fi
  ;;

OSF*:*:alpha )
  if whence ${PBASE}.osf > /dev/null ; then
    execprog ${PBASE}.osf ${PNAME} "${@}"
  fi
  ;;

IRIX*:*:mips )
  if whence ${PBASE}.irix > /dev/null ; then
    execprog ${PBASE}.irix ${PNAME} "${@}"
  fi
  ;;

esac

if whence ${PBASE}.ksh93 > /dev/null ; then
  if [[ -d "${AST}" ]] && [[ -x "${AST}/bin/ksh" ]] ; then
    execprog ${AST}/bin/ksh ksh93 ${PBASE}.ksh93 "${@}"
  fi
fi

if whence ${PBASE}.ksh > /dev/null ; then
  execprog ${PBASE}.ksh ${PNAME} "${@}"
fi

if whence ${PBASE}.sh > /dev/null ; then
  execprog ${PBASE}.sh ${PNAME} "${@}"
fi


print "${PBASE}: could not find underlying program" >&2
exit 1



