#!/usr/extra/bin/ksh
#exec 2> e
#set -x
# PRT - local UNISON 'prt' look-alike program
#
# program to print out files on remote (non-UNISON) printers
#

# Synopsis:
#
#	prt [-d printer] [files(s) ...] [other options ...] [-D]
#
# Notes:
#
#	This program contains KSH specific language elements and it
#	MUST be executed by KSH!
#
#
# revision history:
#
# = 1991-07-01, Dave Morano
#	This program was written from some previous programs that
#	I wrote to handle printing from PCs.
#



# configurable parameters

DEFDST=${LPDEST:-ps}
DST=${PRINTER:=${DEFDST}}

NODE_LP=rca

RF_DEBUG=false


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

: ${NODE:=$( nodename )}
: ${USERNAME:=$( username )}
export NODE USERNAME


# get rid of some bad stuff

unset ENV
unalias cd


# set some environment variables

RF_DST=false
RF_POSTLOCAL=false
RF_DWBHOME=false
RF_DRAFT=false
RF_REVERSE=false

U_PRT=prt

R_PRT=/opt/unison/bin/prt
R_USAGE="usage"
R_TEST="unknown"
R_NODE=hocpb.ho.lucent.com



# some useful programs

P_UNAME=/bin/uname
P_CUT=/bin/cut
P_FGREP=/bin/fgrep
P_DOMAINNAME=/bin/domainname



case ${NODE} in

hocp[a-d] )
  : ${DWBHOME:=/opt/dwb}
  : ${PCS:=/home/gwbb/add-on/pcs}
  : ${LOCAL:=/home/gwbb/add-on/local}
  : ${NCMP:=/home/gwbb/add-on/ncmp}
  : ${TOOLS:=/opt/exptools}
  U_PRT=/opt/unison/bin/prt
  RF_DST=true
  ;;

rc* | hocp[w-z] )
  : ${DWBHOME:=/usr/add-on/dwb}
  : ${PCS:=/usr/add-on/pcs}
  : ${LOCAL:=/usr/add-on/local}
  R_PRT=/opt/unison/bin/prt
  R_NODE=rca.rightcore.com
  RF_DST=true
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


# FUNCTION begin (pathadd)
pathadd() {
  VARNAME=$1
  shift
  if [[ $# -ge 1 ]] && [[ -d "${1}" ]] ; then
    eval AA=\${${VARNAME}}
    print -- ${AA} | ${P_FGREP} "${1}" > /dev/null
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
# FUNCTION end (pathadd)


case ${OSTYPE} in
BSD )
  pathadd PATH /usr/5bin f
  ;;
esac


if [[ -d ${DWBHOME} ]] ; then
  RF_DWBHOME=true
  pathadd PATH ${DWBHOME}/bin f
fi

if [[ -d /usr/ucb ]] ; then
  pathadd PATH /usr/ucb
fi


pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib

pathadd PATH ${NCMP}/bin
pathadd LD_LIBRARY_PATH ${NCMP}/lib

pathadd PATH ${TOOLS}/bin
pathadd LD_LIBRARY_PATH ${TOOLS}/lib

pathadd PATH ${PCS}/bin
pathadd LD_LIBRARY_PATH ${PCS}/lib


if [[ ${R_TEST} == unknown ]] ; then
  R_TEST="${R_PRT} -h"
fi

RF_POSTNEW=false
POSTDIR=${DWBHOME}/bin

case ${OSTYPE} in
BSD )
  RSH=rsh
  ;;
SYSV )
  RSH=remsh
  if [[ -d /usr/sbin ]] ; then 
    RSH=/bin/rsh 
  fi
  ;;
esac


# create proper mail addresses if needed later
MA=${NODE}!${USERNAME}

case ${USERNAME} in
gdb | linda )
  MA=big!${USERNAME}
  ;;
* )
  MA=${NODE}!${USERNAME}
  ;;
esac


# make our log entries

P=`basename ${0}`

LOGFILE=${LOCAL}/log/prt

LOGID=`print -- "${NODE}${$}         " | cut -c 1-14 `

logprint() {
  print -- "${LOGID} ${*}" >> ${LOGFILE}
}


DATE=` date '+%y%m%d_%T' `
logprint "${DATE} ${P} ${VERSION}/${OSTYPE}"

if [[ -n "${FULLNAME}" ]] ; then
  A=${FULLNAME}
else
  if [[ -n "${NAME}" ]] ; then
    A=${NAME}
  else
      A=`userinfo - name`
  fi
fi

if [[ -n "${A}" ]] ; then
  logprint "${MA} (${A})"
else
  logprint "${MA}"
fi


# temporary files??

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

  if [[ ${RF_DEBUG} != true ]] ; then

  case ${SW} in
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
    eval ${RSH} $R_NODE uux -p ${UM}!uuexec ${SJ_CMD}
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
    print -- $SW $IM $UM $SJ_CMD >&2
    eval print -- $SW $IM $UM $SJ_CMD >&2
  fi

}

addo_cvt() {
  if [[ $RF_POSTNEW == true ]] ; then
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

RF_DEBUG=false
RF_INPUT=false

RF_COPIES=false
#RF_DST=false
RF_PLANG=false
RF_MAIL=false
RF_SIDES=false
RF_PMODE=false
RF_FORM=false
RF_BINARY=false
RF_JOBNAME=false
RF_JOBBIN=false
RF_USER=false
RF_QUIET=false
RF_O=false
RF_K=false
RF_L=false

S=files
OS=""
for A in "${@}" ; do
  case ${A} in
  '-D' )
    RF_DEBUG=true
    ;;
  '-B' )
    RF_BINARY=true
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
    print -u2 "${P}: for more HELP see the manual page"
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
    RF_MAIL=true
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
    RF_QUIET=true
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
    RF_INPUT=true
    ;;
  '-'* )
    print -u2 "${P}: unknown option \"${A}\""
    exit 1
    ;;
  * )
    case ${S} in
    files )
      FILES="${FILES} ${A}"
      ;;
    bin )
      RF_JOBBIN=true
      JOBBIN=${A}
      S=${OS}
      ;;
    copies )
      RF_COPIES=true
      COPIES=${A}
      S=${OS}
      ;;
    dst )
      S=${OS}
      RF_DST=true
      case ${A} in
      /*/* )
        DST=${A}
        ;;
      /* )
        DST=`print -- $A | cut -c2-15`
        ;;
      * )
        DST=${A}
        ;;
      esac
      ;;
    form )
      FORM=${A}
      S=${OS}
      RF_FORM=true
      ;;
    jobname )
      JOBNAME=${A}
      RF_JOBNANE=true
      S=${OS}
      ;;
    koptions )
      S=${OS}
      RF_K=true
      KOPTS[ki]="${A}"
      (( ki += 1 ))
      ;;
    loptions )
      S=${OS}
      RF_L=true
      LOPT="${A}"
      ;;
    lang )
      PLANG=${A}
      RF_PLANG=true
      S=${OS}
      ;;
    options )
      S=${OS}
      RF_O=true
      ;;
    pmode )
      S=${OS}
      PMODE=${A}
      RF_PMODE=true
      ;;
    sides )
      RF_SIDES=true
      SIDES=${A}
      S=${OS}
      ;;
    user )
      S=${OS}
      USERNAME=${A}
      RF_USER=true
      ;;
    esac
    ;;
  esac
done


if ${RF_O} ; then
  print -u2 "${P}: option 'o' not supported ; being phased out by UNISON"
fi


HANDLE=default

PRTOPTS=""
  LPOPTS=""
  RSOPTS=""
POSTOPTS=""

# validate the arguments

if ${RF_DST} && [[ -z ${DST} ]] ; then 

  logprint "no printer destination was found"
  print -u2 "${P}: a null printer destination was given"
  exit 1

fi


# if destination is given as PostScript, assume that as the language

logprint "printer=${DST}"

case ${DST} in
hp*ps | gwbb*ps | di*ps )
  if [[ ${RF_PLANG} != true ]] ; then
    PLANG=postscript
    RF_PLANG=true
    DST=`lprootname ${DST} `
  fi
  ;;
esac


if [ -z "${FILES}" ] ; then RF_INPUT=true ; fi


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
  print -u2 "${P}: can't handle \"${PLANG}\", run preprocessor yourself"
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


RF_KILLER=false
if ${RF_FORM} ; then
  case ${DST}:${FORM} in
  *qms:ledger )
    RF_KILLER=true
    ;;
  *:* )
    PRTOPTS="${PRTOPTS} -f ${FORM}"
    ;;
  esac
fi


if ${RF_SIDES} ; then
  if [ "${SIDES}" -gt 2 -o "${SIDES}" -lt 0 ] ; then
    SIDES=2
  fi
  PRTOPTS="${PRTOPTS} -s ${SIDES}"
  logprint "sides=${SIDES}"
fi


if ${RF_PMODE} ; then
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
    print -u2 "${P}: unknown print mode \"${PMODE}\""
    exit 1
    ;;
  esac
  addo_cvt -p ${PMODE}
  PRTOPTS="${PRTOPTS} -p ${PMODE}"
  logprint "printmode=${PMODE}"
fi

if ${RF_MAIL} ; then
  PRTOPTS="${PRTOPTS} -m"
fi

if ${RF_BINARY} ; then
  PRTOPTS="${PRTOPTS} -B"
fi

if ${RF_JOBBIN} ; then
  PRTOPTS="${PRTOPTS} -b ${JOBBIN}"
fi

if ${RF_QUIET} ; then
  PRTOPTS="${PRTOPTS} -q"
fi

if ${RF_USER} ; then
  PRTOPTS="${PRTOPTS} -u ${USERNAME}"
fi

if ${RF_COPIES} ; then
  PRTOPTS="${PRTOPTS} -c ${COPIES}"
fi

#if ${RF_DST} ; then
#
#  PRTOPTS="${PRTOPTS} -d ${DST}"
#
#fi

  if ${RF_COPIES} ; then
    LPOPTS="-n ${COPIES}"
    RSOPTS="-c ${COPIES}"
  fi

  if ${RF_MAIL} ; then
    LPOPTS="${LPOPTS} -m"
    RSOPTS="${RSOPTS} -m"
  fi

  if ${RF_SIDES} ; then
    RSOPTS="${RSOPTS} -s ${SIDES}"
  fi

  if ${RF_USER} ; then
    RSOPTS="${RSOPTS} -u ${USERNAME}"
  fi


# process the K options

RF_DRAFT=0

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
    RF_DRAFT=1
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


# test for compatibility among some options that might conflict!

if [ $RF_DRAFT -ne 0 ] ; then
  case "${PMODE}" in
  land* )
    RF_DRAFT=0
    ;;
  esac
fi


: ${DEVTYPE:=post}
RF_REVERSE=false
RF_GOT=false


# set some LP spooler printer options for some selected printers

case ${DST} in
hp0 | ps )
  DST=hp0
  if ${RF_SIDES} ; then
    LPOPTS="${LPOPTS} -o sides=${SIDES}"
  fi
  if [[ -n "${TRAY}" ]] ; then
    LPOPTS="${LPOPTS} -o tray=${TRAY}"
  fi
  if [[ -n "${DUPLEX}" ]] ; then
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
  if ${RF_FORM} ; then
    case ${FORM} in
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
#	RF_GOT		flag to say whether everything has been determined
#	PHOST		printer host to handle the print job
#	SJ_CMD		command to handle final print submission
#

if ${RF_DST} ; then

  case ${DST} in
  hp[0-9]ps | gwbb[0-9]ps | di[0-9]ps )
    PLANG=postscript
    RF_LANG=true
    DST=`lprootname ${DST} `
    ;;
  esac

  case ${NODE}:${DST} in
  *:lp* )
    HANDLE=lp
    : ${DEVTYPE:=nroff}
    ;;
  rca:hp0 | rca:ps )
    HANDLE=disco
    SW=local
    RF_GOT=true
    PHOST=rca.rightcore.com
    SJ_CMD="lp -s -o nobanner -T postscript -d ${DST} ${LPOPTS}"
    : ${DEVTYPE:=post}
    ;;
  rcb:hp0 | rcb:ps )
    HANDLE=disco
    SW=local
    RF_GOT=true
    PHOST=rca.rightcore.com
    SJ_CMD="lp -s -o nobanner -T postscript -d ${DST} ${LPOPTS}"
    : ${DEVTYPE:=post}
    ;;
  rc*:hp0 | rc*:ps | hocp*:hp0 | hocp*:ps )
    HANDLE=disco
#    SW=local
#    RF_GOT=true
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
    RF_GOT=true
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


if ${RF_DEBUG} ; then

  print -u2 "${P}: printer type is \"${HANDLE}\""
  print -u2 -- "${P}: files-"
  if ${RF_INPUT} ; then
    print -u2 -- "*std_input*"
  else
    for F in ${FILES} ; do
      print -u2  -- "	${F}"
    done
  fi

fi


case ${HANDLE} in
lp )
  if ${RF_INPUT} ; then
    case ${NODE} in
    mtgbcs | mtgzfs3 )
      uux -p ${NODE_LP}!uuexec lp -s -d ${DST}
      ;;
    * )
      $RSH ${R_NODE} uux -p ${NODE_LP}!uuexec lp -s -d ${DST}
      ;;
    esac
  else
    for F in ${FILES} ; do
      if [[ -s $F ]] && [[ ! -d $F ]] ; then
        case ${NODE} in
        mtgbcs | mtgzfs3 )
          uux -C ${NODE_LP}!uuexec lp -s -c -d ${DST} !${F}
          ;;
	* )
          ${RSH} ${R_NODE} uux -p ${NODE_LP}!uuexec lp -s -d ${DST} < $F
          ;;
	esac
      else
        print -u2 -- "${P}: file \"${F}\" is empty or not readable"
      fi
    done
  fi
  ;;
default )
  if ${RF_DST} ; then
    PRTOPTS="${PRTOPTS} -d ${DST}"
  fi
  if ${RF_PLANG} ; then
    PRTOPTS="${PRTOPTS} -l ${PLANG}"
  fi
  case ${PLANG} in
# handle postprocessing that the standard UNISON stuff cannot!
  ras* | tif* | xplot | gplot | p[bgpn]m | pct )
    PRTOPTS="${PRTOPTS} -l postscript"
    CMD="${POSTCVT} ${POSTOPTS}"
    ;;
# handle postprocessing that we want to be local (even just if for fun?)
  troff | troffout )
    PRTOPTS="${PRTOPTS} -l postscript"
    CMD="${POSTCVT} ${POSTOPTS}"
    ;;
# all other stuff can go to the default UNISON stuff
  * )
    if ${RF_PLANG} ; then
      PRTOPTS="${PRTOPTS} -l ${PLANG}"
    fi
    CMD=""
    ;;
  esac
# do it
  if ${RF_INPUT} ; then
    if [[ -x "${U_PRT}" ]] ; then
      if [[ -n "${CMD}" ]] ; then
        if ${RF_KILLER} ; then
          eval ${CMD} | ${U_PRT} ${PRTOPTS} -K "\"TRAY 2\""
        else
          eval ${CMD} | ${U_PRT} ${PRTOPTS}
        fi
      else
        if ${RF_KILLER} ; then
          ${U_PRT} ${PRTOPTS} -K "TRAY 2"
        else
          ${U_PRT} ${PRTOPTS}
        fi
      fi
    else
      ${RSH} -n ${R_NODE} ${R_TEST} 2> /dev/null | fgrep ${R_USAGE} > /dev/null
      if [ $? -eq 0 ] ; then
        if [ -n "${CMD}" ] ; then
          eval ${CMD} | ${RSH} ${R_NODE} ${R_PRT} ${PRTOPTS}
        else
          ${RSH} ${R_NODE} ${R_PRT} ${PRTOPTS}
        fi
      else
        if [[ ${RF_USER} != true ]] ; then
          PRTOPTS="${PRTOPTS} -u ${USERNAME}"
        fi
        if [[ -n "${CMD}" ]] ; then
	  eval ${CMD} | rslow -f ${MA} ${IM} ${R_PRT} ${PRTOPTS}
	else
	  rslow -f ${MA} ${IM} ${R_PRT} ${PRTOPTS}
	fi
      fi
    fi
  else
    if [[ -x "${U_PRT}" ]] ; then
      for F in ${FILES} ; do
        if [[ -s $F ]] && [[ ! -d $F ]] ; then
          if [ -n "${CMD}" ] ; then
        if ${RF_KILLER} ; then
            ${CMD} $F | ${U_PRT} ${PRTOPTS} -K "TRAY 2"
        else
            ${CMD} $F | ${U_PRT} ${PRTOPTS}
        fi
          else
        if ${RF_KILLER} ; then
            ${U_PRT} ${PRTOPTS} $F -K "TRAY 2"
        else
            ${U_PRT} ${PRTOPTS} $F
        fi
          fi
        else
          print -u2 -- "${P}: file \"${F}\" is empty or not readable"
        fi
      done
    else
      ${RSH} -n ${R_NODE} ${R_TEST} | fgrep ${R_USAGE} > /dev/null
      if [[ $? -eq 0 ]] ; then
        for F in ${FILES} ; do
          if [[ -s $F ]] && [[ ! -d $F ]] ; then
            if [[ -n "${CMD}" ]] ; then
              eval ${CMD} $F | ${RSH} ${R_NODE} ${R_PRT} ${PRTOPTS}
            else
              ${RSH} ${R_NODE} ${R_PRT} ${PRTOPTS} < $F
            fi
          else
            print -u2 -- "${P}: file \"${F}\" is empty or not readable"
          fi
        done
      else
        if [[ ${RF_USER} != true ]] ; then
          PRTOPTS="${PRTOPTS} -u ${USERNAME}"
        fi
        for F in ${FILES} ; do
          if [[ -s $F ]] && [[ ! -d $F ]] ; then
	    if [[ -n "${CMD}" ]] ; then
	      eval $CMD | rslow -f ${MA} ${IM} ${R_PRT} ${PRTOPTS}
	    else
	      rslow -f ${MA} ${IM} ${R_PRT} ${PRTOPTS}
	    fi
          else
            print -u2 -- "${P}: file \"${F}\" is empty or not readable"
          fi
        done
      fi
    fi
  fi
  ;;
disco )
  IM=${PHOST}
  UM=$( print -- ${PHOST} | cut -d . -f 1 )

# search for a suitable transport agent if we do not already have one

# get anyone who is one of these special logins

  if [[ ${RF_GOT} != true ]] || [[ "${SW}" != local ]] ; then
    case ${USERNAME} in
    root | special | trouble | uucp | nuucp )
      SW=rslow
      RSOPTS="${RSOPTS} -U"
      SJ_CMD="prt -d ${DST} ${PRTOPTS} -l post "
      RF_GOT=true
      ;;
    esac
  fi

# get eveyone who is left

  if [[ ${RF_GOT} != true ]] ; then

  ${RSH} -n $IM echo YESCODE 2> /dev/null | fgrep YESCODE > /dev/null
  if [[ $? -eq 0 ]] ; then
    SW=rsh
  else
    case $NODE in
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

  if ${RF_DEBUG} ; then
    print -- "prt: dst=${DST} plang=${PLANG}"
    print -- "prt: mach=${NODE}"
  fi >&2

  case ${DST}:${PLANG} in
  hp[0-9]:text | hp[0-9]:printer )
    case ${NODE} in
    rc* | hocp* )
      Y_OFFSET="0.2"
      ;;
    * )
      Y_OFFSET="0.2"
      ;;
    esac
    ;;
  gwbb2:printer | di2:printer )
    case ${NODE} in
    rc* | hocp* )
      if ${RF_DEBUG} ; then print -u2 -- "${0}: mach=${NODE}" ; fi
      Y_OFFSET="0.25"
      ;;
    * )
      if ${RF_DEBUG} ; then print -u2 -- "${0}: mach=${NODE}" ; fi
      ;;
    esac
    ;;
  gwbb2:troff | gwbb2:troffout | di2:troff | di2:troffout )
    case "${NODE}" in
    rc* | hocp* )
      Y_OFFSET="0.1"
      ;;
    * )
      Y_OFFSET="0.1"
      ;;
    esac
    ;;
  esac

  if ${RF_DEBUG} ; then
    print -u2 -- "${0}: yoffset=${Y_OFFSET}"
  fi

  if [ -n "${Y_OFFSET}" ] ; then 
    addo_cvt -y ${Y_OFFSET}
  fi

  case ${DST} in
  gwbb1 | di1 )
    RF_REVERSE=true
    ;;
  esac

# we do it

  case ${PLANG} in
  raw | post* )
    if ${RF_INPUT} ; then
      if $RF_REVERSE ; then
        postreverse | sendjob ${SJ_OPTS}
      else
        sendjob ${SJ_OPTS}
      fi
    else
      if ${RF_REVERSE} ; then
        for F in ${FILES} ; do
          if [[ -s $F ]] && [[ ! -d $F ]] ; then
	    postreverse $F | sendjob ${SJ_OPTS}
          else
            print -u2 -- "${P}: file \"${F}\" is empty or not readable"
          fi
        done
      else
        for F in ${FILES} ; do
          if [[ -s $F ]] && [[ ! -d $F ]] ; then
	    sendjob ${SJ_OPTS} < $F
          else
            print -u2 -- "${P}: file \"${F}\" is empty or not readable"
          fi
        done
      fi
    fi
    ;;
  '-' | txt | text | printer | simple )
    if ${RF_INPUT} ; then
      cat > $TFA
      FILES=$TFA
    fi
      for F in ${FILES} ; do
        if [[ -s $F ]] && [[ ! -d $F ]] ; then
          head -1 $F | grep "^%!PS" > /dev/null
          if [ $? -ne 0 ] ; then
	    CMD="textset -${LINES} ${F}"
            CMD="${CMD} | troff -T${DEVTYPE} | ${POSTCVT} ${POSTOPTS}"
            if [ ${RF_DRAFT} -ne 0 ] ; then
              CMD="${CMD} | postdraft"
            fi
            if ${RF_REVERSE} ; then
              CMD="${CMD} | postreverse"
            fi
            eval ${CMD} | sendjob ${SJ_OPTS}
          else
	    if ${RF_INPUT} ; then
              F="** standard input **"
            fi
            print -u2 -- "${P}: PostScript as text? - file \"${F}\""
          fi
        else
	  if ${RF_INPUT} ; then
            F="** standard input **"
          fi
          print -u2 -- "${P}: file \"${F}\" is empty or not readable"
        fi
      done
    ;;
  troff | troffout | tek | gif | plot )
    if ${RF_INPUT} ; then
      CMD="${POSTCVT} ${POSTOPTS}"
            if [[ ${RF_DRAFT} -ne 0 ]] ; then
              CMD="${CMD} | postdraft"
            fi
      if ${RF_REVERSE} ; then
        CMD="${CMD} | postreverse"
      fi
      eval ${CMD} | sendjob $SJ_OPTS
    else
      for F in ${FILES} ; do
        if [[ -s $F ]] && [[ ! -d $F ]] ; then
	  CMD="${POSTCVT} ${POSTOPTS} ${F}"
            if [[ ! ${RF_DRAFT} ]] ; then
              CMD="${CMD} | postdraft"
            fi
          if ${RF_REVERSE} ; then
            CMD="${CMD} | postreverse"
          fi
          eval ${CMD} | sendjob $SJ_OPTS
        else
          print -u2 -- "${P}: file \"${F}\" is empty or not readable"
        fi
      done
    fi
    ;;
  tif | [gx]plot | pnm | ras | pct | pdf )
    if ${RF_INPUT} ; then
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
            if ${RF_DRAFT} ; then
              CMD="${CMD} | postdraft"
            fi
      if ${RF_REVERSE} ; then
        CMD="${CMD} | postreverse"
      fi
      eval ${CMD} | sendjob $SJ_OPTS
      rm -f $TF
    else
      for F in ${FILES} ; do
        if [[ -s $F ]] && [[ ! -d $F ]] ; then
          case ${PLANG} in
          pdf )
	    CMD="${POSTCVT} ${F} -"
            ;;
          * )
	    CMD="${POSTCVT} ${F}"
            ;;
          esac
            if ${RF_DRAFT} ; then
              CMD="${CMD} | postdraft"
            fi
          if ${RF_REVERSE} ; then
            CMD="${CMD} | postreverse"
          fi
          eval ${CMD} | sendjob ${SJ_OPTS}
        else
          print -u2 -- "${P}: file \"${F}\" is empty or not readable"
        fi
      done
    fi
    ;;
  * ) 
    print -u2 -- "${P}: specified language is not supported"
    exit 1
    ;;
  esac
  ;;
esac


cleanup



