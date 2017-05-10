#!/bin/ksh
# XXX (some general program)


ADDON_ECE=/proj/arch1/dmorano/add-on
ADDON_URI=/abroad/morano/add-on


PROG_WHICH=/bin/which
PROG_DOMAINNAME=/bin/domainname
PROG_FGREP=/bin/fgrep
PROG_CUT=/bin/cut
PROG_TR=/bin/tr
PROG_UNAME=/bin/uname
PROG_DIRNAME=/bin/dirname
PROG_BASENAME=/bin/basename


function getinetdomain {
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
  NODENAME=$( ${PROG_UNAME} -n )
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
  for A ; do
    if [[ -d "${A}" ]] ; then
      export $1
    fi
  done
}

okexport LOCAL AST NCMP


OS_SYSTEM=$( ${PROG_UNAME} -s )
OS_RELEASE=$( ${PROG_UNAME} -r )
ARCHITECTURE=$( ${PROG_UNAME} -p )


case ${OS_SYSTEM}:${OS_RELEASE}:${ARCHITECTURE} in

SunOS:5.*:sparc )
  MACHINE=$( ${PROG_UNAME} -m )
  case $MACHINE in

  sun4u )
    ARCHITECTURE="${ARCHITECTURE}u"
    ;;

  esac
  ;;

esac

case ${OS_SYSTEM}:${OS_RELEASE}:${ARCHITECTURE} in

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

OFF=$( print $OFD | ${PROG_TR} '[A-Z]' '[a-z]' )


addpath() {
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


# get our program name

PDIR=$( ${PROG_DIRNAME} ${0} )
A=${0##*/}
PNAME=${A%.*}


# add the package area BIN to the users PATH

addpath PATH ${NCMP}/bin f
addpath PATH ${AST}/bin f
addpath PATH ${LOCAL}/bin f

addpath PATH ${HOME}/bin

if [[ -d "${PDIR}" ]] ; then
  addpath PATH ${PDIR}
fi


addpath LD_LIBRARY_PATH ${NCMP}/lib/${OFD} f
addpath LD_LIBRARY_PATH ${AST}/lib f
addpath LD_LIBRARY_PATH ${LOCAL}/lib/${OFD} f

addpath LD_LIBRARY_PATH ${HOME}/lib/${OFD}


# find the EXECNAME program if we can

EXECNAME=${LOCAL}/bin/execname
if [ ! -x $EXECNAME ] ; then EXECNAME=${LOCAL}/etc/bin/execname ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=${NCMP}/bin/execname ; fi

if [ ! -x $EXECNAME ] ; then EXECNAME=execname ; fi



# function to EXEC a program, the best that we can

execprog() {
  EXECPROG_PATH=$1
  shift
  EXECPROG_NAME=$1
  shift
  if whence $EXECNAME > /dev/null ; then
    exec $EXECNAME $EXECPROG_PATH $EXECPROG_NAME "${@}"
  else
    exec $EXECPROG_PATH "${@}"
  fi
}





# run the program intended, finally

case ${OS_SYSTEM}:${OS_RELEASE}:${ARCHITECTURE} in

SunOS:5.[789]*:sparcu )
  if whence ${PNAME}.s5u > /dev/null ; then
    execprog ${PNAME}.s5u ${PNAME} "${@}"
  fi

  if whence ${PNAME}.us5 > /dev/null ; then
    execprog ${PNAME}.us5 ${PNAME} "${@}"
  fi

  if whence ${PNAME}.s5 > /dev/null ; then
    execprog ${PNAME}.s5 ${PNAME} "${@}"
  fi

  if whence ${PNAME}.elf > /dev/null ; then
    execprog ${PNAME}.elf ${PNAME} "${@}"
  fi

  if whence ${PNAME}.x > /dev/null ; then
    execprog ${PNAME}.x ${PNAME} "${@}"
  fi

  if whence ${PNAME}.s4 > /dev/null ; then
    execprog ${PNAME}.s4 ${PNAME} "${@}"
  fi

  if whence ${PNAME}.aout > /dev/null ; then
    execprog ${PNAME}.aout ${PNAME} "${@}"
  fi
  ;;

SunOS:5*:sparcu )
  if whence ${PNAME}.us5 > /dev/null ; then
    execprog ${PNAME}.us5 ${PNAME} "${@}"
  fi

  if whence ${PNAME}.s5 > /dev/null ; then
    execprog ${PNAME}.s5 ${PNAME} "${@}"
  fi

  if whence ${PNAME}.elf > /dev/null ; then
    execprog ${PNAME}.elf ${PNAME} "${@}"
  fi

  if whence ${PNAME}.x > /dev/null ; then
    execprog ${PNAME}.x ${PNAME} "${@}"
  fi

  if whence ${PNAME}.s4 > /dev/null ; then
    execprog ${PNAME}.s4 ${PNAME} "${@}"
  fi

  if whence ${PNAME}.aout > /dev/null ; then
    execprog ${PNAME}.aout ${PNAME} "${@}"
  fi
  ;;

SunOS:5*:sparc* )
  if whence ${PNAME}.s5 > /dev/null ; then
    execprog ${PNAME}.s5 ${PNAME} "${@}"
  fi

  if whence ${PNAME}.elf > /dev/null ; then
    execprog ${PNAME}.elf ${PNAME} "${@}"
  fi

  if whence ${PNAME}.x > /dev/null ; then
    execprog ${PNAME}.x ${PNAME} "${@}"
  fi

  if whence ${PNAME}.s4 > /dev/null ; then
    execprog ${PNAME}.s4 ${PNAME} "${@}"
  fi

  if whence ${PNAME}.aout > /dev/null ; then
    execprog ${PNAME}.aout ${PNAME} "${@}"
  fi
  ;;

SunOS:4*:sparc )
  if whence ${PNAME}.s4 > /dev/null ; then
    execprog ${PNAME}.s4 ${PNAME} "${@}"
  fi

  if whence ${PNAME}.aout > /dev/null ; then
    execprog ${PNAME}.aout ${PNAME} "${@}"
  fi

  if whence ${PNAME}.x > /dev/null ; then
    execprog ${PNAME}.x ${PNAME} "${@}"
  fi
  ;;

OSF*:*:alpha )
  if whence ${PNAME}.osf > /dev/null ; then
    execprog ${PNAME}.osf ${PNAME} "${@}"
  fi
  ;;

IRIX*:*:mips )
  if whence ${PNAME}.irix > /dev/null ; then
    execprog ${PNAME}.irix ${PNAME} "${@}"
  fi
  ;;

esac

if whence ${PNAME}.ksh93 > /dev/null ; then
  if [[ -d "${AST}" ]] && [[ -x "${AST}/bin/ksh" ]] ; then
    execprog ${AST}/bin/ksh ksh93 ${PNAME}.ksh93 "${@}"
  fi
fi

if whence ${PNAME}.ksh > /dev/null ; then
  execprog ${PNAME}.ksh ${PNAME} "${@}"
fi

if whence ${PNAME}.sh > /dev/null ; then
  execprog ${PNAME}.sh ${PNAME} "${@}"
fi


print "${PNAME}: could not find underlying program" >&2
exit 1



