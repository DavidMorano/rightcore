#!/usr/extra/bin/ksh
# PRTFMT - local PRTFMT look-alike program
#
# program to print out files on remote (non-UNISON) printers
#

#
#
# 1992-07-01, David A.D. Morano
# - original write
#
#
# 1993-01-10, David A.D. Morano
# - enhanced to make more common across various machine/print platforms
#
# 2018-02-14, David A.D. Morano
#	Refactored to get rid of OSTYPE.  And refactoed to switch to the 
#	intrisic version of the KSH "test" facility, instead of using the
#	KSH built-in 'test' command.
#


# configurable defaults


# get rid of some bad stuff

unset ENV
#unalias cd

# OK to continue

RF_POSTNEW=0
RF_PRINTER=0

P_WHICH=/bin/which
P_FGREP=/bin/fgrep
P_DOMAINNAME=/bin/domainname
P_CUT=/bin/cut
P_UNAME=/bin/uname
P_BASENAME=/bin/basename
P_MKDRAFT=mkdraft
P_DPOST=dpost
P_PRT=prt

: ${HOME:=$( userhome )}
: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
: ${EXTRA:=/usr/extra}
export HOME LOCAL NCMP PCS EXTRA

PRS=" ${HOME} ${LOCAL} ${EXTRA} "

if [[ "${FPATH:0:1}" == ":" ]] ; then
  FPATH=${FPATH:1:200}
fi

for PR in ${PRS} ; do
  if [[ -d ${PR} ]] ; then
    B=${PR}/fbin
    if [[ -d ${B} ]] ; then
      if [[ -n "${FPATH}" ]] ; then
        FPATH="${FPATH}:${B}"
      else
        FPATH=${B}
      fi
    fi
  fi
done
export FPATH

for PR in ${PRS} ; do
  pathadd PATH ${PR}/bin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done


: ${NODE:=$( nodename )}
: ${OSTYPE:=$( sysval ostype )}
export NODE OSTYPE

DN=/dev/null
PN=${0##*/}


case ${OSTYPE} in
BSD )
  P_RSH=rsh
  ;;
SYSV )
  P_RSH=remsh
  if [[ -d /usr/sbin ]] ; then 
    P_RSH=/bin/rsh 
  fi
  ;;
esac


haveprog() {
  ES1=1
  ${P_WHICH} $1 | ${P_FGREP} "no " | ${P_FGREP} "in " > ${DN}
  ES=$?
  if [[ ${ES} -eq 0 ]] ; then ES1=1 ; else ES1=0 ; fi
  return ${ES1}
}

add_dpopts() {
  if [[ ${RF_POSTNEW} -ne 0 ]] ; then
    O_DPOST="${O_DPOST} ${1} ${2}"
  else
    O_DPOST="${O_DPOST} ${1}${2}"
  fi
}

addcmd() {
  if [[ -z "${CMD}" ]] ; then
    CMD="${1}"
  else
    CMD="${CMD} | ${1}"
  fi
}


# the machine entries below may contain:
#
#	RF_PRINTER	an explicit printer was specified
#	PRINTER		default printer destination
#


case ${NODE} in
rc* )
  : ${LOCAL:=/usr/add-on/local}
  : ${DWBHOME:=/usr/add-on/dwb}
  PATH=${LOCAL}/bin:${DWBHOME}/bin:${PATH}
  ;;
* )
  ;;
esac


if [[ -z "${BIBLIOGRAPHY}" ]] ; then
  case ${NODE} in
  rc* | hodi* | hosb* )
    BIBLIOGRAPHY=/proj/starbase/tools/share/bib/INDEX
    ;;
  mt* )
    BIBLIOGRAPHY=/mt/mtgzfs8/hw/tools/lib/papers/local
    ;;
  esac
fi

# end of configurable defaults


# OK, we are ready to start the main program

: ${LOCAL:=/usr/add-on/local}
: ${DWBHOME:=/usr/add-on/dwb}
export LOCAL DWBHOME


RF_DWBHOME=0
if [[ -d "${DWBHOME}" ]] ; then
  RF_DWBHOME=1
  PATH=${DWBHOME}/bin:${PATH}
fi


prepend() {
  cat "${@}"
  cat
}


RF_POSTNEW=0
if [[ ${RF_DWBHOME} -ne 0 ]] ; then
  if whence ${P_DPOST} | fgrep ${DWBHOME} > ${DN} ; then
    RF_POSTNEW=1
  fi
else
  if whence ${P_DPOST} > ${DN} ; then :
  else
    if [[ -x /usr/lib/lp/postscript/dpost ]] ; then
      PATH=${PATH}:/usr/lib/lp/postscript
      P_DPOST=dpost
    fi
  fi
fi


integer ki=0
integer i=0

KOPTS[0]=
O_FMT=""
O_PRT=""
O_DPOST=""

SIDES=2
FORM=nohole
FILES=""
PMODE="port"
TROFFTYPE=
DRAFT=
FMTSPEC=

RF_DEBUG=false
RF_DRAFT=0
RF_INPUT=0
RF_GETOPT=1
RF_FALL=0
RF_PS=0

RF_COPIES=0
RF_TROFFTYPE=0
RF_ZMODE=0
RF_V=0
RF_MAIL=0
RF_PMODE=0
RF_SIDES=0
RF_FORM=0
RF_QUIET=0
RF_FORMATTER=0
RF_HELP=0

S=files
OS=""
for A in "$@" ; do

#print-u2 "ARG ${A}"

  if [[ ${RF_GETOPT} -ne 0 ]] ; then

    case ${A} in
  
    '-DRAFT' )
      RF_DRAFT=1
      DRAFT=3
      ;;
  
    '-DRAFT='* )
      RF_DRAFT=1
      DRAFT=$( print $A | cut -d '=' -f 2 )
      ;;
  
    '-D' )
      RF_DEBUG=true
      ;;
  
    '-F' )
      OS=${S}
      S=formatter
      RF_GETOPT=0
      ;;
  
    '-T' )
      OS=${S}
      S=devtype
      RF_GETOPT=0
      ;;
  
    '-Z' )
      RF_ZMODE=1
      ;;
  
    '-K' )
      OS=${S}
      S=koptions
      ;;
  
    '-c' )
      OS=${S}
      S=copies
      RF_GETOPT=0
      ;;
  
    '-d' )
      OS=${S}
      S=dst
      RF_GETOPT=0
      ;;
  
    '-f' )
      OS=${S}
      S=form
      RF_GETOPT=0
      ;;
  
    '-h' )
      RF_HELP=1
      ;;
  
    '-m' )
      RF_MAIL=1
      ;;
  
    '-p' )
      OS=${S}
      S=pmode
      RF_GETOPT=0
      ;;
  
    '-q' )
      RF_QUIET=1
      ;;
  
    '-s' )
      OS=${S}
      S=sides
      RF_GETOPT=0
      ;;
  
    '-v' )
      RF_V=1
      ;;
  
    '-' )
      RF_INPUT=1
      ;;
  
    '-'* )
      print -u2 "${PN}: unknown option encountered ; use UNISON version"
      exit 1
      ;;
  
    * )
      RF_FALL=1
      ;;
  
    esac
  
  else

    RF_FALL=1

  fi

  if [[ ${RF_FALL} -ne 0 ]] ; then

    RF_FALL=0
    case ${S} in

    files )
      FILES="${FILES} ${A}"
      ;;

    devtype )
      S=${OS}
      RF_TROFFTYPE=1
      TROFFTYPE=${A}
      ;;

    dst )
      S=${OS}
      RF_PRINTER=1
      case ${A} in

      /*/* )
        PRINTER=${A}
        ;;

      /* )
        PRINTER=$( print $A | cut -c2-15 )
        ;;

      * )
        PRINTER=${A}
        ;;

      esac
      ;;

    copies )
      S=${OS}
      COPIES=${A}
      RF_COPIES=1
      ;;

    form )
      S=${OS}
      FORM=${A}
      RF_FORM=1
      ;;

    formatter )
      S=${OS}
      RF_FORMATTER=1
      FMTSPEC=${A}
      ;;

    pmode )
      S=${OS}
      PMODE=${A}
      RF_PMODE=1
      ;;

    sides )
      SIDES=${A}
      RF_SIDES=1
      S=${OS}
      ;;

    koptions )
      S=${OS}
      RF_K=true
      KOPTS[ki]="${A}"
      (( ki += 1 ))
      ;;

    esac

    RF_GETOPT=1

  fi

done

if [[ ${RF_HELP} -ne 0 ]] ; then
  print -u2 "${PN}: see manual pages"
  exit 1
fi


# establish a default printer
: ${PRINTER:=$( prtdb -d default use )}
export PRINTER

if ${RF_DEBUG} ; then
  print -u2 "PRINTER=${PRINTER}"
fi

if [[ -z "${FILES}" ]] ; then 
  RF_INPUT=1
fi

if [[ ${RF_V} -ne 0 ]] ; then
  O_FMT="${O_FMT} -v"
fi

if [[ ${RF_SIDES} -ne 0 ]] ; then
  if [[ "${SIDES}" -gt 2 ]] || [[ "${SIDES}" -lt 0 ]] ; then 
    SIDES=2 
  fi
  O_PRT="${O_PRT} -s ${SIDES}"
fi

if [[ ${RF_COPIES} -ne 0 ]] ; then
  O_PRT="${O_PRT} -c ${COPIES}"
fi

if [[ ${RF_FORM} -ne 0 ]] ; then
  case "${FORM}" in
  vg | hole | nohole | legal | ledger | library )
    ;;
  8x11 )
    FORM=nohole
    ;;
  11x17 )
    FORM=ledger
    ;;
  14x17 )
    FORM=library
    ;;
  8x14 )
    FORM=legal
    ;;
  * )
    FORM=nohole
    ;;
  esac
  O_PRT="${O_PRT} -f ${FORM}"
fi

case "${PMODE}" in
l* )
  PMODE=landscape
  ;;
p* )
  PMODE=portrait
  ;;
2on1 )
  ;;
* )
  print -u2 "${PN}: unknown print mode \"${PMODE}\""
  exit 1
  ;;
esac

if [[ ${RF_QUIET} -ne 0 ]] ; then
  O_PRT="${O_PRT} -Q"
fi


# handle any formatter options that we may have

FORMATTER=troff
if [[ ${RF_FORMATTER} -ne 0 ]] ; then

  print "${FMTSPEC}" | read O J 

  case "${O}" in

  [a-zA-Z]* )
    FORMATTER=${O}
    O_FMT="${O_FMT} ${J}"
    ;;

  esac

fi

if [[ ${RF_DRAFT} -ne 0 ]] ; then
  case "${DRAFT}" in

  0 | 1 | 2 | 3 | 4 | 5 )
    O_FMT="${O_FMT} -rC${DRAFT}"
    ;;

  esac
fi


(( i = 0 ))
while [[ i -lt ki ]] ; do

  O_PRT="${O_PRT} -K ${KOPTS[i]}"

  (( i += 1 ))
done


# end of command line option pre-processing


# establish program specific variables based on printer destination
#
#	The HANDLE variable allows for different processing
#	based on a handle name.  Handle names should usually be
#	'local' for normal processing on the local machine
#	but can be something else for other printers that
#	provide their own processing of some sort.
#
#	The TROFFTYPE variable specifies the type of printer class
#	that the output should conform to.  This is usually 'post'
#	for all PostScript class printers (the typical case).
#

HANDLE=default


O_PRINTER=""
if [[ ${RF_PRINTER} -ne 0 ]] ; then
  O_PRINTER="-d ${PRINTER}"
fi


: ${TROFFTYPE:=$( prtdb -d ${PRINTER} trofftype )}


# OK, we have finally decided to do some real printing here!!

TF1=/tmp/pfa${$}
TF2=/tmp/pfb${$}

cleanup() {
  rm -f ${TF1} ${TF2}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


if [[ ${RF_INPUT} -ne 0 ]] ; then
      cat > ${TF1}
      FILES="${FILES} ${TF1}"
fi


case ${HANDLE} in

default | local )
  if [[ ${RF_PMODE} -ne 0 ]] ; then
    O_PRT="${O_PRT} -p ${PMODE}"
    add_dpopts -p ${PMODE}
  fi

  if [[ "${RF_DWBHOME}" -eq 0 ]] ; then
        print -u2 "${PN}: no DWB package configured on this machine"
        exit 1
  fi

  SAVE_O_DPOST=${O_DPOST}

  for F in ${FILES} ; do

    O_DPOST=${SAVE_O_DPOST}
    XOFFSET=""
    YOFFSET=""
    RF_BOUNDING=0

    if [[ -r $F ]] ; then

      RF_PIC=0
      RF_MS=0
      RF_PICTURES=0
      RF_COLOR=0
      RF_PS=0

      PREPEND=""
      O_FILE=${TF2}
      CMD=""
      MACPKGS=""

      soelim ${F} > ${TF2}

      AWK_PROG='/^\.[A-Za-z]|^\.\[/ { print substr($1,2,9) }'
      MACS=$( awk "${AWK_PROG}" ${TF2} 2> ${DN} | sort -u )

# eliminate the newlines

      MACS=$( print ${MACS} )
#print -u2 "macs are ${MACS}"

# properly separate all fields uniformly including the first and last

      MACS=" ${MACS} "
#print $MACS > macs
      if ${RF_DEBUG} ; then
        print -u2 "${MACS}"
      fi

# conditionally call in order 'tag', 'grap', 'gc2pic', 'pic', 'tbl', 'eqn'

      case "${MACS}" in
      *so* )
        addcmd "${CMD} soelim ${O_FILE}"
        O_FILE=""
        ;;
      esac

      case "${MACS}" in
      *'[ '* )
        RO=""
        fgrep '$LIST$' ${TF2} > ${DN}
        if [[ $? -eq 0 ]] ; then RO="-e" ; fi
        if [[ -s "${BIBLIOGRAPHY}" ]] ; then
          addcmd "refer ${RO} -p ${BIBLIOGRAPHY} ${O_FILE}"
        else
          addcmd "refer ${RO} ${O_FILE}"
        fi
        O_FILE=""
        ;;
      esac

      case "${MACS}" in
      *IBR* | *BK* )
        if [[ -s "${BIBLIOGRAPHY}" ]] ; then
          addcmd "referm -p ${BIBLIOGRAPHY} ${O_FILE}"
        else
          addcmd "referm ${O_FILE}"
        fi
        O_FILE=""
        ;;
      esac

      case "${MACS}" in
      *Ii* | *BG* )
        addcmd "incima ${O_FILE}"
        O_FILE=""
        RF_PICTURES=1
        ;;
      esac

      case "${MACS}" in
      *vS* )
        if [[ -n "${O_FILE}" ]] ; then
          addcmd "vgrind -f -w < ${O_FILE}"
          O_FILE=""
        else
          addcmd "vgrind -f -w"
        fi
        ;;
      esac

      case "${MACS}" in
      *TA* )
        addcmd "gtag ${O_FILE}"
        O_FILE=""
        ;;
      esac

      case "${MACS}" in
      *G1* )
        addcmd "grap ${O_FILE}"
        RF_PIC=1
        O_FILE=""
        ;;
      esac

      case "${MACS}" in
      *GS* )
        addcmd "gc2pic ${O_FILE}"
        RF_PIC=1
        O_FILE=""
        ;;
      esac

      if [[ ${RF_PIC} -ne 0 ]] ; then
        addcmd "pic ${O_FILE}"
        O_FILE=""
      else
        case "${MACS}" in
        *PS* )
          addcmd "pic ${O_FILE}"
          O_FILE=""
          ;;
        esac
      fi

      case "${MACS}" in
      *TS* )
        addcmd "tbl ${O_FILE}"
        O_FILE=""
        ;;
      esac

      case "${MACS}" in
      *EQ* )
        addcmd "eqn ${O_FILE}"
        O_FILE=""
        ;;
      esac

      O_FMTMAC=""

      case "${MACS}" in
      *VG* )
        O_FMTMAC="-mview"
        ;;
      *PP*T[PH]* | *SH*T[PH]* )
#print -u2 "MAN"
        O_FMTMAC="-man"
        MACPKGS="${MACPKGS} an"
        ;;
      *V[SwhWH]* | *S[whW]* )
#print -u2 "mv - V<x> or SW"
        O_FMTMAC="-mv"
        MACPKGS="${MACPKGS} v"
        ;;
      *AF* | *' H '* | *MT* | *' P '* | *S[KP]* | *[ABVD]L* )
        O_FMTMAC="-mm"
        MACPKGS="${MACPKGS} m"
        addcmd "mmcite ${O_FILE}"
        O_FILE=""
        ;;
      *TL* | *NH* | *[SN]H*PP* | *M[RF]*PP* | *LT*PP* )
        O_FMTMAC="-ms"
        MACPKGS="${MACPKGS} s"
        ;;
      *SH*' T '* )
#print -u2 "mv - SH and T"
        O_FMTMAC="-mv"
        MACPKGS="${MACPKGS} v"
        ;;
      *' A '*SH* | *' C '*SH* | *' D '*SH* )
#print -u2 "mv - SH and [ACD]"
        O_FMTMAC="-mv"
        MACPKGS="${MACPKGS} v"
        ;;
      *dN* | *f[CD]* | *wP* )
        O_FMTMAC="-mcs"
        MACPKGS="${MACPKGS} cs"
        ;;
      *P[FH]* | *' S '* )
        O_FMTMAC="-mm"
        MACPKGS="${MACPKGS} m"
        ;;
      *[RLP]P* | *[TI]M* | *TL* | *[SN]H* | *M[RF]* | *EG* | *LT* )
        O_FMTMAC="-ms"
        MACPKGS="${MACPKGS} s"
        ;;
      *E[FH]* )
        O_FMTMAC="-ms"
        MACPKGS="${MACPKGS} s"
        ;;
      *[DF]S* )
        O_FMTMAC="-mm"
        MACPKGS="${MACPKGS} m"
        ;;
      *IP* )
        O_FMTMAC="-ms"
        MACPKGS="${MACPKGS} s"
        RF_MS=1
        ;;
      *' B '*SH* )
#print -u2 "mv - SH and B"
        O_FMTMAC="-mv"
        MACPKGS="${MACPKGS} v"
        ;;
      *[pil]p* )
        O_FMTMAC="-me"
        MACPKGS="${MACPKGS} e"
        ;;
# I think that the following may have to always be last
      *' P '* | *' R '* | *' B '* | *' I '* )
        O_FMTMAC="-mm"
        MACPKGS="${MACPKGS} m"
        ;;
      esac

      if [[ ${RF_MS} -ne 0 ]] ; then
        case "${MACS}" in
        *FL* | *FC* | *KF* | *P[123]* | *SP* | *Tm* | *' X '* )
          O_FMTMAC="${O_FMTMAC} -mpm"
          MACPKGS="${MACPKGS} pm"
          ;;
        esac
      fi

      case "${MACS}" in
      *BP* | *PI* )
        RF_PICTURES=1
        ;;
      esac

      case "${MACS}" in
      *CL* )
        RF_COLOR=1
        ;;
      esac

      case "${MACS}" in
      *lM* | pM* )
        O_FMTMAC="${O_FMTMAC} -mps"
        MACPKGS="${MACPKGS} ps"
        RF_PS=1
        ;;
      esac

      if [[ ${RF_PS} -eq 0 ]] && [[ ${RF_PICTURES} -ne 0 ]] ; then
        O_FMTMAC="${O_FMTMAC} -mpictures"
        MACPKGS="${MACPKGS} pictures"
# the following causes problems with shifting of the printed image
#            RF_BOUNDING=1
      fi

      if [[ ${RF_PS} -eq 0 ]] && [[ ${RF_COLOR} -ne 0 ]] ; then
        O_FMTMAC="${O_FMTMAC} -mcolor"
        MACPKGS="${MACPKGS} color"
      fi

# PostScript was included?

      if [[ ${RF_PS} -eq 0 ]] || [[ ${RF_PICTURES} -ne 0 ]] ; then
        addcmd "psboxsize ${O_FILE}"
        O_FILE=""
      fi

# permuted index macros

      case "${MACS}" in
      *xx* )
        O_FMTMAC="${O_FMTMAC} -mptx"
        MACPKGS="${MACPKGS} ptx"
        ;;
      esac

# other macro packages that should be prepended to the document

# the old "indent" command seems to be gone!
#
#         case "${MACS}" in
#         *HD* | *Fn* | *Pr* | *De* | *Du* )
#            TMAC_INDENT=/usr/share/lib/tmac/tmac.indent
#            if [[ -r $TMAC_INDENT ]] ; then
#              PREPEND="${PREPEND} /usr/share/lib/tmac/tmac.indent"
#            fi
#            ;;
#         esac
#

# fix up some problems based on the type of document that we know about

      YOFFSET=$( prtdb -d $PRINTER yoffset )

if ${RF_DEBUG} ; then
  print -u2 "YOFFSET=${YOFFSET}"
fi

      case "${MACPKGS}" in
      *' an'* )
        XOFFSET="0.1"
        ;;
      *' e'* )
#print -u2 "ME macros offset"
        XOFFSET="0.8"
        ;;
      *' m'* | *' s'* )
        case "${PMODE}" in
        port* )
          XOFFSET="0.25"
          ;;
        esac
        ;;
      esac

      if [[ "${RF_BOUNDING}" -ne 0 ]] ; then
        O_DPOST="${O_DPOST} -B"
      fi

      if [[ -n "${XOFFSET}" ]] ; then
        add_dpopts -x $XOFFSET
      fi

      if [[ -n "${YOFFSET}" ]] ; then
        add_dpopts -y $YOFFSET
      fi

# do we have some macro package to prepend to the file?

      if [[ -n "${PREPEND}" ]] ; then
        if [[ -n "${O_FILE}" ]] ; then
          addcmd "prepend ${PREPEND} < ${O_FILE}"
          O_FILE=""
        else
          addcmd "prepend ${PREPEND}"
        fi
      fi

# OK, put the line with the formatter in place together

      O_DT=
      if [[ -n "${TROFFTYPE}" ]] ; then
        O_DT="-T${TROFFTYPE}"
      fi

if ${RF_DEBUG} ; then
  print -u2 "O_FMTMAC=${O_FMTMAC}"
fi

      if [[ ${RF_V} -ne 0 ]] ; then
        addcmd "${FORMATTER} ${O_DT} ${O_FMTMAC} ${O_FMT}"
      else
        addcmd "${FORMATTER} ${O_DT} ${O_FMTMAC} ${O_FMT} ${O_FILE}"
      fi

if ${RF_DEBUG} ; then
  print -u2 "CMD=${CMD}"
fi

# OK, prepare final form and go!

      if [[ ${RF_ZMODE} -eq 0 ]] ; then
        addcmd "${P_DPOST} ${O_DPOST}"
        if [[ ${RF_DRAFT} -ne 0 ]] ; then
          CMD="${CMD} | ${P_MKDRAFT}"
        fi
        CMD="${CMD} | ${P_PRT} $O_PRINTER -l post ${O_PRT}"
      fi

      if ${RF_DEBUG} || [[ ${RF_V} -ne 0 ]] ; then
        print -u2 ">${CMD}<"
        eval ${CMD}
      else :
        eval ${CMD}
      fi

    else
      print -u2 "${PN}: file \"${F}\" not readable"
    fi

  done
  ;;

esac

cleanup


