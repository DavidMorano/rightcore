#!/usr/bin/ksh
# PRT - local UNISON 'prt' look-alike program
#
# program to print out files on remote (non-UNISON) printers
#

# Synopsis :
#
#	prt [-d printer] [files(s) ...] [other options ...] [-D]
#
# Notes:
#
#	This program contains KSH specific language elements and there
#	MUST be executed by KSH !
#
#
#
# revision history:
#
# = 91/07/01, Dave Morano
#	This program was written from some previous programs that
#	I wrote to handle printing from PCs.
#



# configurable parameters

DEFDST=${LPDEST:-ps}
DST=${PRINTER:=${DEFDST}}

MACH_LP=rca


# program specific things
#
VERSION=0

#
# IMPORTANT NOTE: On Use Of The Environment Variable 'LANG'
#
# Do NOT be tempted to ever use an environment variable named 'LANG'.
# This severely conflicts with the newer standard UNIXes that use
# this for some sort of local language specification.
#

# program starts here

if [ -d /usr/sbin ] ; then
  OSTYPE=SYSV
  MACH=`uname -n`
else
  OSTYPE=BSD
  MACH=`hostname`
fi


# get rid of some bad stuff

unset ENV
unalias cd


# set some environment variables

F_DST=false
F_POSTLOCAL=false
F_DWBHOME=false

U_PRT=prt
R_PRT=/opt/unison/bin/prt
R_USAGE="usage"
R_TEST="unknown"
R_MACH=hocpb.ho.lucent.com



# some useful programs

P_UNAME=/bin/uname
P_CUT=/bin/cut
P_FGREP=/bin/fgrep
P_DOMAINNAME=/bin/domainname



case ${MACH} in

hocp[a-d] )
  : ${DWBHOME:=/opt/dwb}
  : ${PCS:=/home/gwbb/add-on/pcs}
  : ${LOCAL:=/home/gwbb/add-on/local}
  : ${NCMP:=/home/gwbb/add-on/ncmp}
  : ${TOOLS:=/opt/exptools}
  U_PRT=/opt/unison/bin/prt
  F_DST=true
  ;;

rc* | hocp[w-z] )
  : ${DWBHOME:=/usr/add-on/dwb}
  : ${PCS:=/usr/add-on/pcs}
  : ${LOCAL:=/usr/add-on/local}
  R_PRT=/opt/unison/bin/prt
  R_MACH=rca.rightcore.com
  F_DST=true
  ;;

* )
  ;;

esac

# end of configurables


# start of psuedo regular non-changeable program

: ${DWBHOME:=/usr/add-on/dwb}
: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
: ${TOOLS:=/usr/add-on/exptools}
export DWBHOME LOCAL NCMP PCS TOOLS



addpath() {
  VARNAME=$1
  shift
  if [ $# -ge 1 -a -d "${1}" ] ; then
    eval AA=\${${VARNAME}}
    echo ${AA} | ${P_FGREP} "${1}" > /dev/null
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
  unset VARNAME AA
}


case $OSTYPE in

BSD )
  addpath PATH /usr/5bin f
  ;;

esac


if [ -d ${DWBHOME} ] ; then
  F_DWBHOME=true
  addpath PATH ${DWBHOME}/bin f
fi

if [ -d /usr/ucb ] ; then
  addpath PATH /usr/ucb
fi


addpath PATH ${LOCAL}/bin f
addpath PATH ${NCMP}/bin
addpath PATH ${TOOLS}/bin
addpath PATH ${PCS}/bin



if [ ${R_TEST} = unknown ] ; then
  R_TEST="${R_PRT} -h"
fi

F_POSTNEW=false
POSTDIR=${DWBHOME}/bin

case ${OSTYPE} in

BSD )
  RSH=rsh
  ;;

SYSV )
  RSH=remsh
  if [ -d /usr/sbin ] ; then 
    RSH=/bin/rsh 
  fi
  ;;

esac


# create proper mail addresses if needed later

if [ -z "${LOGNAME}" ] ; then

  if [ -n "${USER}" ] ; then 
    LOGNAME=${USER}
  fi

  USERNAME=${LOGNAME}
  if [ -z "${LOGNAME}" ] ; then 
    LOGNAME=pcs 
    USERNAME=nobody
  fi
  export LOGNAME

else

  USERNAME=${LOGNAME}

fi

MA=${MACH}!${LOGNAME}

case ${LOGNAME} in

gdb | linda )
  MA=big!${LOGNAME}
  ;;

* )
  MA=${MACH}!${LOGNAME}
  ;;

esac


# make our log entries

P=`basename ${0}`

LOGFILE=${LOCAL}/log/prt

LOGID=`echo "${MACH}${$}         " | cut -c 1-14 `

logprint() {
  echo "${LOGID} ${*}" >> $LOGFILE
}


DATE=` date '+%y%m%d_%T' `
logprint "${DATE} ${P} ${VERSION}/${OSTYPE}"

if [ -n "${FULLNAME}" ] ; then
  A=${FULLNAME}
else
  if [ -n "${NAME}" ] ; then
    A=${NAME}
  else
      A=`logname - name`
  fi
fi

if [ -n "${A}" ] ; then
  logprint "${MA} (${A})"
else
  logprint "${MA}"
fi



# temporary files ??

TFA=/tmp/prta${$}
TFB=/tmp/prtb${$}

cleanup() {
  rm -f $TFA $TFB
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17



# try to fix up PATH

# send a job to the DiSCO printers

sendjob() {

  SW=$1
  IM=$2
  UM=$3

  if [ $F_DEBUG != true ] ; then

  case $SW in

  rex )
    AF=${LOCAL}/etc/prt/gwbb
    if [ -f $AF ] ; then

      OPTS="-a ${AF}"

    else

      OPTS="-u guest -p guest*"

    fi

    eval rex $OPTS $IM $SJ_CMD 
    ;;

  rsh )
    eval ${RSH} $IM $SJ_CMD 
    ;;

  rsh_uux )
    eval ${RSH} $R_MACH uux -p ${UM}!uuexec ${SJ_CMD}
    ;;

  uux )
    eval uux -p ${UM}!uuexec ${SJ_CMD}
    ;;

  rslow )
    eval rslow -f $MA -u $USERNAME $IM $SJ_CMD
    ;;

  local )
    eval $SJ_CMD
    ;;

  esac

  else

    cat > /dev/null
    echo $SW $IM $UM $SJ_CMD >&2
    eval echo $SW $IM $UM $SJ_CMD >&2

  fi

}

addo_cvt() {
  if [ $F_POSTNEW = true ] ; then
    POSTOPTS="${POSTOPTS} ${1} ${2}"
  else
    POSTOPTS="${POSTOPTS} ${1}${2}"
  fi
}



integer ki=0
integer i=0

KOPTS[0]=
FILES=""
JOBBIN=""
COPIES=1
SIDES=1
PLANG=text
PMODE=portrait
FORM=nohole

F_DEBUG=false
F_INPUT=false

F_COPIES=false
#F_DST=false
F_PLANG=false
F_MAIL=false
F_SIDES=false
F_PMODE=false
F_FORM=false
F_BINARY=false
F_JOBNAME=false
F_JOBBIN=false
F_USER=false
F_QUIET=false
F_O=false
F_K=false
F_L=false

S=files
OS=""
for A in "${@}" ; do

  case $A in

  '-D' )
    F_DEBUG=true
    ;;

  '-B' )
    F_BINARY=true
    ;;

  '-K' )
    OS=${S}
    S=koptions
    ;;

  '-L' )
    OS=${S}
    S=loptions
    ;;

  '-b' )
    OS=${S}
    S=bin
    ;;

  '-c' )
    OS=${S}
    S=copies
    ;;

  '-d' )
    OS=${S}
    S=dst
    ;;

  '-f' )
    OS=${S}
    S=form
    ;;

  '-h' )
    echo "${P}: for more HELP see the manual page" >&2
    ;;

  '-j' )
    OS=${S}
    S=jobname
    ;;

  '-l' )
    OS=${S}
    S=lang
    ;;

  '-m' )
    F_MAIL=true
    ;;

  '-p' )
    OS=$S
    S=pmode
    ;;

  '-o' )
    OS=${S}
    S=options
    ;;

  '-q' )
    F_QUIET=true
    ;;

  '-s' )
    OS=${S}
    S=sides
    ;;

  '-u' )
    OS=${S}
    S=user
    ;;

  '-' )
    F_INPUT=true
    ;;

  '-'* )
    echo "${P}: unknown option \"${A}\"" >&2
    exit 1
    ;;

  * )
    case $S in

    files )
      FILES="${FILES} ${A}"
      ;;

    bin )
      F_JOBBIN=true
      JOBBIN=${A}
      S=${OS}
      ;;

    copies )
      F_COPIES=true
      COPIES=${A}
      S=${OS}
      ;;

    dst )
      S=${OS}
      F_DST=true
      case $A in

      /*/* )
        DST=${A}
        ;;

      /* )
        DST=`echo $A | cut -c2-15`
        ;;

      * )
        DST=${A}
        ;;

      esac
      ;;

    form )
      FORM=${A}
      S=${OS}
      F_FORM=true
      ;;

    jobname )
      JOBNAME=${A}
      F_JOBNANE=true
      S=${OS}
      ;;

    koptions )
      S=${OS}
      F_K=true
      KOPTS[ki]="${A}"
      (( ki += 1 ))
      ;;

    loptions )
      S=${OS}
      F_L=true
      LOPT="${A}"
      ;;

    lang )
      PLANG=${A}
      F_PLANG=true
      S=${OS}
      ;;

    options )
      S=${OS}
      F_O=true
      ;;

    pmode )
      S=${OS}
      PMODE=${A}
      F_PMODE=true
      ;;

    sides )
      F_SIDES=true
      SIDES=${A}
      S=${OS}
      ;;

    user )
      S=${OS}
      USERNAME=${A}
      F_USER=true
      ;;

    esac
    ;;

  esac

done


if [ $F_O = true ] ; then
  echo "${P}: option 'o' not supported ; being phased out by UNISON" >&2
fi


HANDLE=default

PRTOPTS=""
  LPOPTS=""
  RSOPTS=""
POSTOPTS=""

# validate the arguments

if [ ${F_DST} = true -a -z ${DST} ] ; then 

  logprint "no printer destination was found"
  echo "${P}: a null printer destination was given" >&2
  exit 1

fi


# if destination is given as PostScript, assume that as the language

logprint "printer=${DST}"

case $DST in

hp*ps | gwbb*ps | di*ps )
  if [ $F_PLANG != true ] ; then

    PLANG=postscript
    F_PLANG=true
    DST=`lprootname ${DST} `

  fi
  ;;

esac


if [ -z "${FILES}" ] ; then F_INPUT=true ; fi


case "${FORM}" in

vg | hole | nohole | legal | library | ledger )
  ;;

8[xX]11 )
  FORM=nohole
  ;;

11[xX]17 )
  FORM=ledger
  ;;

8[xX]14 )
  FORM=legal
  ;;

14[xX]17 )
  FORM=library
  ;;

* )
  FORM=nohole
  ;;

esac

logprint "form=${FORM}"


case "${PLANG}" in

troff | troffout )
  POSTCVT=${POSTDIR}/dpost
  ;;

post* )
  PLANG=postscript
  POSTCVT=""
  ;;

printer | txt | text | simple )
  PLANG=text
  POSTCVT=${POSTDIR}/dpost
  ;;

troffin | pictpost )
  echo "${P}: can't handle \"${PLANG}\", run preprocessor yourself" >&2
  exit 1
  ;;

tek )
  POSTCVT=${POSTDIR}/posttek
  ;;

gif )
  POSTCVT=${POSTDIR}/postgif
  ;;

plot )
  POSTCVT=${POSTDIR}/postplot
  ;;

gplot )
  POSTCVT=plot2ps
  ;;

xplot )
  POSTCVT=p4topost
  ;;

p[bgpn]m )
  PLANG=pnm
  POSTCVT=pnmtops
  ;;

tif* )
  PLANG=tif
  POSTCVT=tiff2ps
  ;;

ras* )
  PLANG=ras
  POSTCVT=ras2ps
  ;;

pct | pict )
  PLANG=pct
  POSTCVT=pct2ps
  ;;

pdf )
  POSTCVT=pdftops
  ;;

esac

logprint "language=${PLANG}"


F_KILLER=false
if [ ${F_FORM} = true ] ; then

  case ${DST}:${FORM} in

  *qms:ledger )
    F_KILLER=true
    ;;

  *:* )
    PRTOPTS="${PRTOPTS} -f ${FORM}"
    ;;

  esac

fi


if [ ${F_SIDES} = true ] ; then
  if [ "${SIDES}" -gt 2 -o "${SIDES}" -lt 0 ] ; then
    SIDES=2
  fi
  PRTOPTS="${PRTOPTS} -s ${SIDES}"

  logprint "sides=${SIDES}"

fi


if [ ${F_PMODE} = true ] ; then

  case "${PMODE}" in

  p* )
    PMODE=portrait
    ;;

  l* )
    PMODE=landscape
    ;;

  2on1 )
    ;;

  * )
    echo "${P}: unknown print mode \"${PMODE}\"" >&2
    exit 1
    ;;

  esac

  addo_cvt -p ${PMODE}
  PRTOPTS="${PRTOPTS} -p ${PMODE}"

  logprint "printmode=${PMODE}"

fi

if [ ${F_MAIL} = true ] ; then
  PRTOPTS="${PRTOPTS} -m"
fi

if [ ${F_BINARY} = true ] ; then
  PRTOPTS="${PRTOPTS} -B"
fi

if [ ${F_JOBBIN} = true ] ; then
  PRTOPTS="${PRTOPTS} -b ${JOBBIN}"
fi

if [ ${F_QUIET} = true ] ; then
  PRTOPTS="${PRTOPTS} -q"
fi

if [ ${F_USER} = true ] ; then
  PRTOPTS="${PRTOPTS} -u ${USERNAME}"
fi

if [ ${F_COPIES} = true ] ; then
  PRTOPTS="${PRTOPTS} -c ${COPIES}"
fi

#if [ ${F_DST} = true ] ; then
#
#  PRTOPTS="${PRTOPTS} -d ${DST}"
#
#fi

  if [ ${F_COPIES} = true ] ; then
    LPOPTS="-n ${COPIES}"
    RSOPTS="-c ${COPIES}"
  fi

  if [ $F_MAIL = true ] ; then
    LPOPTS="${LPOPTS} -m"
    RSOPTS="${RSOPTS} -m"
  fi

  if [ $F_SIDES = true ] ; then
    RSOPTS="${RSOPTS} -s ${SIDES}"
  fi

  if [ $F_USER = true ] ; then
    RSOPTS="${RSOPTS} -u ${USERNAME}"
  fi


# process the K options

F_DRAFT=0

TRAY=
DUPLEX=

(( i = 0 ))
while [[ i -lt ki ]] ; do

  print ${KOPTS[i]} | fgrep '=' > /dev/null
  RS=$?
  if [ $RS -eq 0 ] ; then
    KEY=$( print ${KOPTS[i]} | cut -d '=' -f 1 )
    VALUE=$( print ${KOPTS[i]} | cut -d '=' -f 2 )
  else
    print ${KOPTS[i]} | read KEY VALUE
  fi

  case $KEY in

  TRAY | tray )
    case $VALUE in

    1 | u* | t* )
      TRAY=upper
      ;;

    2 | l* | b* )
      TRAY=lower
      ;;

    esac

    logprint "option tray=${TRAY}"
    ;;

  DRAFT | draft )
    F_DRAFT=1
    logprint "option draft"
    ;;

  DUPLEX | duplex )
    case $VALUE in

    duplex | on | double )
      DUPLEX=duplex
      ;;

    simplex | off | single )
      DUPLEX=simplex
      ;;

    short | long | tablet | standard | vd* | hd* )
      DUPLEX=${VALUE}
      ;;

    esac
    ;;

  SIMPLEX | simplex )
    DUPLEX=simplex
    ;;

  esac

  (( i += 1 ))

done


if [ -n "${DUPLEX}" ] ; then
  logprint "option duplex=${DUPLEX}"
fi


# test for compatibility among some options that might conflict !

if [ $F_DRAFT -ne 0 ] ; then
  case "${PMODE}" in

  land* )
    F_DRAFT=0
    ;;

  esac
fi



: ${DEVTYPE:=post}
F_REVERSE=false
F_GOT=false


# set some LP spooler printer options for some selected printers

case ${DST} in

hp0 | ps )
  DST=hp0
  if [ $F_SIDES = true ] ; then
    LPOPTS="${LPOPTS} -o sides=${SIDES}"
  fi
  if [ -n "${TRAY}" ] ; then
    LPOPTS="${LPOPTS} -o tray=${TRAY}"
  fi
  if [ -n "${DUPLEX}" ] ; then
    LPOPTS="${LPOPTS} -o duplex=${DUPLEX}"
  fi
  ;;

ep | ep0 )
  DST=ep0
  ;;

gwbb1 )
  DST=gwbb1
  ;;

esac

case ${DST} in

di2* | di3* )
  if [ $F_FORM = true ] ; then

    case $FORM in

    *hole | 8[xX]11 | 11[xX]17 | letter | ledger | tabloid )
      LPOPTS="${LPOPTS} -o papersize=${FORM}"
      ;;

    esac

  fi
  ;;

esac


# continue with machine/printer specific information

#
#	We want to set some variables for each printer destination.
#	HANDLE		who should handle this destination
#	SW		how (which delivery program) is is handled
#	F_GOT		flag to say whether everything has been determined
#	PHOST		printer host to handle the print job
#	SJ_CMD		command to handle final print submission
#

if [ ${F_DST} = true ] ; then

  case ${DST} in

  hp[0-9]ps | gwbb[0-9]ps | di[0-9]ps )
    PLANG=postscript
    F_LANG=true
    DST=`lprootname $DST `
    ;;

  esac

  case ${MACH}:${DST} in

  *:lp* )
    HANDLE=lp
    : ${DEVTYPE:=nroff}
    ;;

  rca:hp0 | rca:ps )
    HANDLE=disco
    SW=local
    F_GOT=true
    PHOST=rca.rightcore.com
    SJ_CMD="lp -s -o nobanner -T postscript -d ${DST} ${LPOPTS}"
    : ${DEVTYPE:=post}
    ;;

  rcb:hp0 | rcb:ps )
    HANDLE=disco
    SW=local
    F_GOT=true
    PHOST=rca.rightcore.com
    SJ_CMD="lp -s -o nobanner -T postscript -d ${DST} ${LPOPTS}"
    : ${DEVTYPE:=post}
    ;;

  rc*:hp0 | rc*:ps | hocp*:hp0 | hocp*:ps )
    HANDLE=disco
#    SW=local
#    F_GOT=true
    PHOST=rca.rightcore.com
    SJ_CMD="lp -s -o nobanner -T postscript -d ${DST} ${LPOPTS}"
    : ${DEVTYPE:=post}
    ;;

  *:hp | *:hp0 | *:ps | *:di0 )
    HANDLE=disco
    PHOST=rca.rightcore.com
    SJ_CMD="lp -s -o nobanner -T postscript -d ${DST} ${LPOPTS}"
    : ${DEVTYPE:=post}
    ;;

  rcb:ep0 | rcb:ep )
    HANDLE=disco
    SW=local
    F_GOT=true
    PHOST=rcb.rightcore.com
    SJ_CMD="lp -s -o nobanner -T postscript -d ${DST} ${LPOPTS}"
    : ${DEVTYPE:=post}
    ;;

  rc*:rp0 | rc*:rp | hocp*:ep0 | hocp*:ep )
    HANDLE=disco
    PHOST=rcb.rightcore.com
    SJ_CMD="lp -s -o nobanner -T postscript -d ${DST} ${LPOPTS}"
    : ${DEVTYPE:=post}
    ;;

  *:ep0 | *:ep )
    HANDLE=disco
    PHOST=hocpx.ho.lucent.com
    SJ_CMD="lp -s -o nobanner -T postscript -d ${DST} ${LPOPTS}"
    : ${DEVTYPE:=post}
    ;;

  default )
    ;;

  null )
    HANDLE=null
    ;;

  * )
    ;;

  esac

fi

logprint "handle=${HANDLE}"


if [ ${F_DEBUG} = true ] ; then

  echo "${P}: printer type is \"${HANDLE}\"" >&2
  echo "${P}: files -" >&2
  if [ $F_INPUT = true ] ; then
    echo "*std_input*" >&2
  else
    for F in $FILES ; do
      echo "	${F}" >&2
    done
  fi

fi


case ${HANDLE} in

lp )
  if [ ${F_INPUT} = true ] ; then

    case ${MACH} in

    mtgbcs | mtgzfs3 )
      uux -p ${MACH_LP}!uuexec lp -s -d ${DST}
      ;;

    * )
      $RSH ${R_MACH} uux -p ${MACH_LP}!uuexec lp -s -d ${DST}
      ;;

    esac

  else

    for F in ${FILES} ; do

      if [ -s $F -a ! -d $F ] ; then

        case ${MACH} in

        mtgbcs | mtgzfs3 )
          uux -C ${MACH_LP}!uuexec lp -s -c -d ${DST} !${F}
          ;;

	* )
          ${RSH} ${R_MACH} uux -p ${MACH_LP}!uuexec lp -s -d ${DST} < $F
          ;;

	esac

      else

        echo "${P}: file \"${F}\" is empty or not readable" >&2

      fi

    done

  fi
  ;;

default )
  if [ $F_DST = true ] ; then
    PRTOPTS="${PRTOPTS} -d ${DST}"
  fi

  if [ ${F_PLANG} = true ] ; then
    PRTOPTS="${PRTOPTS} -l ${PLANG}"
  fi

  case ${PLANG} in

# handle postprocessing that the standard UNISON stuff cannot !

  ras* | tif* | xplot | gplot | p[bgpn]m | pct )
    PRTOPTS="${PRTOPTS} -l postscript"
    CMD="${POSTCVT} ${POSTOPTS}"
    ;;

# handle postprocessing that we want to be local (even just if for fun ?)

  troff | troffout )
    PRTOPTS="${PRTOPTS} -l postscript"
    CMD="${POSTCVT} ${POSTOPTS}"
    ;;

# all other stuff can go to the default UNISON stuff

  * )
    if [ ${F_PLANG} = true ] ; then
      PRTOPTS="${PRTOPTS} -l ${PLANG}"
    fi
    CMD=""
    ;;

  esac

# do it

  if [ ${F_INPUT} = true ] ; then

    if [ -x "${U_PRT}" ] ; then

      if [ -n "${CMD}" ] ; then

        if [ $F_KILLER = true ] ; then
        eval ${CMD} | ${U_PRT} ${PRTOPTS} -K "\"TRAY 2\""
        else
        eval ${CMD} | ${U_PRT} ${PRTOPTS}
        fi

      else

        if [ $F_KILLER = true ] ; then
        ${U_PRT} ${PRTOPTS} -K "TRAY 2"
        else
        ${U_PRT} ${PRTOPTS}
        fi

      fi

    else

      ${RSH} -n ${R_MACH} ${R_TEST} 2> /dev/null | fgrep ${R_USAGE} > /dev/null
      if [ $? -eq 0 ] ; then

        if [ -n "${CMD}" ] ; then

          eval ${CMD} | ${RSH} ${R_MACH} ${R_PRT} ${PRTOPTS}

        else

          ${RSH} ${R_MACH} ${R_PRT} ${PRTOPTS}

        fi

      else

        if [ ${F_USER} != true ] ; then

          PRTOPTS="${PRTOPTS} -u ${LOGNAME}"

        fi

        if [ -n "${CMD}" ] ; then

	  eval $CMD | rslow -f ${MA} ${IM} ${R_PRT} ${PRTOPTS}

	else

	  rslow -f ${MA} ${IM} ${R_PRT} ${PRTOPTS}

	fi

      fi

    fi

  else

    if [ -x "${U_PRT}" ] ; then

      for F in ${FILES} ; do

        if [ -s $F -a ! -d $F ] ; then

          if [ -n "${CMD}" ] ; then

        if [ $F_KILLER = true ] ; then
            ${CMD} $F | ${U_PRT} ${PRTOPTS} -K "TRAY 2"
        else
            ${CMD} $F | ${U_PRT} ${PRTOPTS}
        fi

          else

        if [ $F_KILLER = true ] ; then
            ${U_PRT} ${PRTOPTS} $F -K "TRAY 2"
        else
            ${U_PRT} ${PRTOPTS} $F
        fi

          fi

        else

          echo "${P}: file \"${F}\" is empty or not readable" >&2

        fi

      done

    else

      ${RSH} -n ${R_MACH} ${R_TEST} | fgrep ${R_USAGE} > /dev/null
      if [ $? -eq 0 ] ; then

        for F in ${FILES} ; do

          if [ -s $F -a ! -d $F ] ; then

            if [ -n "${CMD}" ] ; then

              eval ${CMD} $F | ${RSH} ${R_MACH} ${R_PRT} ${PRTOPTS}

            else

              ${RSH} ${R_MACH} ${R_PRT} ${PRTOPTS} < $F

            fi

          else

            echo "${P}: file \"${F}\" is empty or not readable" >&2

          fi

        done

      else

        if [ ${F_USER} != true ] ; then

          PRTOPTS="${PRTOPTS} -u ${LOGNAME}"

        fi

        for F in ${FILES} ; do

          if [ -s $F -a ! -d $F ] ; then

	    if [ -n "${CMD}" ] ; then

	      eval $CMD | rslow -f ${MA} ${IM} ${R_PRT} ${PRTOPTS}

	    else

	      rslow -f ${MA} ${IM} ${R_PRT} ${PRTOPTS}

	    fi

          else

            echo "${P}: file \"${F}\" is empty or not readable" >&2

          fi

        done

      fi

    fi

  fi
  ;;

disco )
  IM=$PHOST
  UM=`echo $PHOST | cut -d . -f 1 `

# search for a suitable transport agent if we do not already have one

# get anyone who is one of these special logins

  if [ $F_GOT != true -o "${SW}" != local ] ; then

    case $LOGNAME in

    root | special | trouble | uucp | nuucp )
      SW=rslow
      RSOPTS="${RSOPTS} -U"
      SJ_CMD="prt -d ${DST} ${PRTOPTS} -l post "
      F_GOT=true
      ;;

    esac

  fi

# get eveyone who is left

  if [ $F_GOT != true ] ; then

  ${RSH} -n $IM echo YESCODE 2> /dev/null | fgrep YESCODE > /dev/null
  if [ $? -eq 0 ] ; then

    SW=rsh

  else

    case $MACH in

    rc* | hocp[w-z] )
      SW=local
      ;;

    hocp* )
      SW=rex
      ;;

    hodi* | octet | dds | disco | boole | mtgbcs | mtgzfs3 | mtsvi )
      SW=uux
      ;;

    mthost* | mtgbs* | mtsol | mtgzfs[0-9]* | mtgz[0-9]* | mtnor )
      SW=rsh_uux
      ;;

    * )
      SW=rslow
      ;;

    esac

  fi

  fi

# end if (of getting eveyone)

  logprint "transport=${SW}"


  SJ_OPTS="${SW} ${IM} ${UM}"

  LINES=60
  Y_OFFSET=""

  if [ $F_DEBUG = true ] ; then

    echo "prt: dst=${DST} plang=${PLANG}"
    echo "prt: mach=${MACH}"

  fi >&2

  case ${DST}:${PLANG} in

  hp[0-9]:text | hp[0-9]:printer )
    case ${MACH} in

    rc* | hocp* )
      Y_OFFSET="0.2"
      ;;

    * )
      Y_OFFSET="0.2"
      ;;

    esac
    ;;

  gwbb2:printer | di2:printer )
    case ${MACH} in

    rc* | hocp* )
      if [ $F_DEBUG = true ] ; then echo "prt: mach=${MACH}" >&2 ; fi
      Y_OFFSET="0.25"
      ;;

    * )
      if [ $F_DEBUG = true ] ; then echo "prt: mach=${MACH}" >&2 ; fi
      ;;

    esac
    ;;

  gwbb2:troff | gwbb2:troffout | di2:troff | di2:troffout )
    case "${MACH}" in

    rc* | hocp* )
      Y_OFFSET="0.1"
      ;;

    * )
      Y_OFFSET="0.1"
      ;;

    esac
    ;;

  esac

  if [ $F_DEBUG = true ] ; then

    echo "prt: yoffset=${Y_OFFSET}"

  fi >&2

  if [ -n "${Y_OFFSET}" ] ; then 
    addo_cvt -y ${Y_OFFSET}
  fi

  case ${DST} in

  gwbb1 | di1 )
    F_REVERSE=true
    ;;

  esac

# we do it

  case ${PLANG} in

  raw | post* )
    if [ ${F_INPUT} = true ] ; then

      if [ $F_REVERSE = true ] ; then

        postreverse | sendjob $SJ_OPTS

      else

        sendjob $SJ_OPTS

      fi

    else

      if [ ${F_REVERSE} = true ] ; then

        for F in ${FILES} ; do

          if [ -s $F -a ! -d $F ] ; then

	    postreverse $F | sendjob $SJ_OPTS

          else

            echo "${P}: file \"${F}\" is empty or not readable" >&2

          fi

        done

      else

        for F in ${FILES} ; do

          if [ -s $F -a ! -d $F ] ; then

	    sendjob $SJ_OPTS < $F

          else

            echo "${P}: file \"${F}\" is empty or not readable" >&2

          fi

        done

      fi

    fi
    ;;

  '-' | txt | text | printer | simple )
    if [ ${F_INPUT} = true ] ; then

      cat > $TFA
      FILES=$TFA

    fi

      for F in ${FILES} ; do

        if [ -s $F -a ! -d $F ] ; then

          head -1 $F | grep "^%!PS" > /dev/null
          if [ $? -ne 0 ] ; then

	    CMD="textset -${LINES} ${F}"
            CMD="${CMD} | troff -T${DEVTYPE} | ${POSTCVT} ${POSTOPTS}"

            if [ ${F_DRAFT} -ne 0 ] ; then
              CMD="${CMD} | postdraft"
            fi

            if [ ${F_REVERSE} = true ] ; then
              CMD="${CMD} | postreverse"
            fi

            eval ${CMD} | sendjob $SJ_OPTS

          else

	    if [ $F_INPUT = true ] ; then
              F="** standard input **"
            fi

            echo "${P}: PostScript as text ? - file \"${F}\"" >&2

          fi

        else

	  if [ $F_INPUT = true ] ; then
            F="** standard input **"
          fi

          echo "${P}: file \"${F}\" is empty or not readable" >&2

        fi

      done

    ;;

  troff | troffout | tek | gif | plot )
    if [ ${F_INPUT} = true ] ; then

      CMD="${POSTCVT} ${POSTOPTS}"

            if [ ${F_DRAFT} -ne 0 ] ; then
              CMD="${CMD} | postdraft"
            fi

      if [ ${F_REVERSE} = true ] ; then
        CMD="${CMD} | postreverse"
      fi

      eval ${CMD} | sendjob $SJ_OPTS

    else

      for F in ${FILES} ; do

        if [ -s $F -a ! -d $F ] ; then

	  CMD="${POSTCVT} ${POSTOPTS} ${F}"

            if [ ${F_DRAFT} -ne 0 ] ; then
              CMD="${CMD} | postdraft"
            fi

          if [ ${F_REVERSE} = true ] ; then
            CMD="${CMD} | postreverse"
          fi

          eval ${CMD} | sendjob $SJ_OPTS

        else

          echo "${P}: file \"${F}\" is empty or not readable" >&2

        fi

      done

    fi
    ;;

  tif | [gx]plot | pnm | ras | pct | pdf )
    if [ ${F_INPUT} = true ] ; then

      TF=/tmp/prt${RANDOM}${$}
      case $PLANG in

      pdf )
        cat > $TF
	CMD="${POSTCVT} ${TF} -"
        ;;

      * )
	CMD="${POSTCVT} "
        ;;

      esac

            if [ ${F_DRAFT} -ne 0 ] ; then
              CMD="${CMD} | postdraft"
            fi

      if [ ${F_REVERSE} = true ] ; then
        CMD="${CMD} | postreverse"
      fi

      eval ${CMD} | sendjob $SJ_OPTS

      rm -f $TF

    else

      for F in ${FILES} ; do

        if [ -s $F -a ! -d $F ] ; then

          case $PLANG in

          pdf )
	    CMD="${POSTCVT} ${F} -"
            ;;

          * )
	    CMD="${POSTCVT} ${F}"
            ;;

          esac

            if [ ${F_DRAFT} -ne 0 ] ; then
              CMD="${CMD} | postdraft"
            fi

          if [ ${F_REVERSE} = true ] ; then
            CMD="${CMD} | postreverse"
          fi

          eval ${CMD} | sendjob $SJ_OPTS

        else

          echo "${P}: file \"${F}\" is empty or not readable" >&2

        fi

      done

    fi
    ;;

  * ) 
    echo "${P}: specified language is not supported" >&2
    exit 1
    ;;

  esac
  ;;

esac


cleanup



