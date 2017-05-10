#!/usr/bin/ksh
# MAKEBELOW


# set some environment variables

ADDON_ECE=/home/research/dmorano/add-on
ADDON_URI=/marlin/morano/add-on



P_WHICH=/bin/which
P_DOMAINNAME=/bin/domainname
P_FGREP=/bin/fgrep
P_CUT=/bin/cut
P_UNAME=/bin/uname
P_BASENAME=/bin/basename


RESOLVE=/etc/resolv.conf


getinetdomain() {
  if [[ -n "${LOCALDOMAIN}" ]] ; then
    for D in ${LOCALDOMAIN} ; do
      print -- $D
      break
    done
  else
    if [[ -r ${RESOLVE} ]] ; then
      ${P_FGREP} domain ${RESOLVE} | while read J1 D J2 ; do
        print -- $D
      done
    else
      ${P_DOMAINNAME}
    fi
  fi
}


: ${NODE:=$( ${P_UNAME} -n | ${P_CUT} -d . -f 1 )}

case ${NODE} in

gilmore* | hammett* )
  DOMAIN=ece.neu.edu
  ;;

* )
  DOMAIN=$( getinetdomain )
  ;;

esac

export NODE DOMAIN


case ${NODE}.${DOMAIN} in

*.uri.edu )
  : ${LOCAL:=${ADDON_URI}/local}
  : ${NCMP:=${ADDON_URI}/ncmp}
  : ${PCS:=${ADDON_URI}/pcs}
  ;;

*.coe.neu.edu )
  : ${LOCAL:=${HOME}}
  : ${NCMP:=${HOME}}
  : ${PCS:=${HOME}}
  ;;

*.ece.neu.edu )
  : ${LOCAL:=${ADDON_ECE}/local}
  : ${NCMP:=${ADDON_ECE}/ncmp}
  : ${PCS:=${ADDON_ECE}/pcs}
  ;;

sparc*.ece.neu.edu )
  ;;

esac


: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
export LOCAL NCMP PCS


: ${OSNAME:=$( ${P_UNAME} -s )}
: ${OSREL:=$( ${P_UNAME} -r )}
: ${ARCHITECTURE:=$( ${P_UNAME} -p )}


case ${OS_SYSTEM}:${OS_RELEASE}:${ARCH} in

SunOS:5.*:sparc )
  PLATFORM=`${P_UNAME} -i`
  case $PLATFORM in

  *Ultra* )
    ARCH="${ARCH}u"
    ;;

  esac
  ;;

esac

case ${OS_SYSTEM}:${OS_RELEASE}:${ARCH} in

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

OFF=$( print -- ${OFD} | tr '[A-Z]' '[a-z]' )


pathadd() {
  VARNAME=$1
  shift
  if [ $# -ge 1 -a -d "${1}" ] ; then
    eval AA=\${${VARNAME}}
    print -- ${AA} | ${P_FGREP} "${1}" > /dev/null
    if [ $? -ne 0 ] ; then
      if [ -z "${AA}" ] ; then
          AA=${1}
      else
        if [ $# -eq 1 ] ; then
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
  unset VARNAME
}


# add the package area BIN to the user's PATH

pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib

pathadd PATH ${NCMP}/bin
pathadd LD_LIBRARY_PATH ${NCMP}/lib

pathadd PATH ${PCS}/bin
pathadd LD_LIBRARY_PATH ${PCS}/lib

pathadd PATH ${HOME}/bin
pathadd LD_LIBRARY_PATH ${HOME}/lib

if [ ! -d /usr/sbin ] ; then PATH=/usr/5bin:${PATH} ; fi


PN=$( basename ${0} )


# temporary files

TARGETS=
DIRS=



RF_DEBUG=0
RF_QUIET=0
RF_TARGETS=0
RF_DIRS=0


S=targets
OS=""
for A in "${@}" ; do

  case $A in

  '-D' )
    RF_DEBUG=true
    ;;

  '-t' )
    OS=${S}
    S=atargets
    ;;

  '-d' )
    OS=${S}
    S=dirs
    ;;

  '-q' )
    RF_QUIET=1
    ;;

  '-'* )
    print -u2 -- "${PN}: unknown option \"${A}\""
    exit 1
    ;;

  * )
    case $S in

    targets )
      RF_TARGETS=1
      TARGETS="${TARGETS} ${A}"
      ;;

    atargets )
      RF_TARGETS=1
      TARGETS="${TARGETS} ${A}"
      S=${OS}
      ;;

    dirs )
      RF_DIRS=1
      DIRS="${DIRS} ${A}"
      S=${OS}
      ;;

    esac
    ;;

  esac

done


# validate the arguments

if [ ${RF_TARGETS} == 0 ] ; then
  TARGETS="upincs up"
fi

if [ ${RF_DIRS} == 0 ] ; then
  ls | while read F ; do
    if [[ -d "${F}" ]] && [[ -r ${F}/Makefile ]] ; then
      DIRS="${DIRS} ${F}"
    fi
  done
fi



#print -u2 TARGETS=${TARGETS} DIRS=${DIRS} 


TF=${TMPDIR:=/tmp}/mb${RANDOM}${$}

cleanup() {
  rm -f $TF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


for T in ${TARGETS} ; do
  for D in ${DIRS} ; do
    if [[ -r ${D}/Makefile ]] ; then (
      cd $D
      make $T
      ES=$?
      if [[ ${ES} -ne 0 ]] ; then
        print -- ${ES} > $TF
        exit ${ES}
      fi
    ) ; if [[ -s ${TF} ]] ; then
        ES=$( cat ${TF} )
        cleanup
        exit ${ES}
      fi
    fi
  done
done

cleanup



