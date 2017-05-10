#!/bin/sh
# MKROFF = make a print job roffable
#
# program to print out files on remote (non-UNISON) printers

# Dave Morano, 07/92
# - original write
#
# Dave Morano, 01/93
# - enhanced to make more common across various machine/print platforms
#


# configurable defaults


# get rid of some bad stuff

unset ENV
#unalias cd

# OK to continue

if [ ! -d /usr/sbin ] ; then

  ARCH=`/bin/arch`
  MACH=`hostname`

else

  ARCH=att
  MACH=`uname -n`

fi


F_POSTLOCAL=false
F_POSTNEW=false
DPOST=dpost

case ${ARCH} in

sun )
  RSH=rsh
  ;;

att )
  RSH=remsh
  if [ -d /usr/sbin ] ; then 

    RSH=/bin/rsh 

  fi
  ;;

esac

add_dpopts() {
  if [ $F_POSTNEW = true ] ; then

    DPOPTS="${DPOPTS} ${1} ${2}"

  else

    DPOPTS="${DPOPTS} ${1}${2}"

  fi
}


P=`basename ${0}`


F_DST=false

unset UNISON
DST=gwbb0
R_SERVER=""
PRT=prt
PRTFMT=prtfmt
F_DWBHOME=false


# the machine entries below may contain :
#	R_SERVER	name of remote UNISON printer server machine
#	PRT		name of 'prt' program
#	PRTFMT		name of underlying 'prtfmt' program
#	UNISON		path to UNISON root directory tree
#	DST		default printer destination
#	F_DST		whether supplied DST variable is used as default



case ${MACH} in

hocp[a-d] | nucleus | logicgate | nitrogen )
# comment out UNISON for the gerneral Suns since it is too buggy to use
#  UNISON=/usr/add-on/unison
  DST=${PRINTER:=1h515ps}
  : ${LOCAL:=/home/gwbb/add-on/local}
  DWBHOME=/opt/dwb
  PATH=${LOCAL}/bin:${DWBHOME}/bin:${PATH}
  ;;

hocp[w-z] )
  R_SERVER=hocpb
  R_PRTFMT=/opt/unison/bin/prtfmt
  DST=${PRINTER:=gwbb0}
  F_DST=true
  : ${LOCAL:=/usr/add-on/local}
  PATH=${LOCAL}/bin:${DWBHOME}/bin:${PATH}
  ;;

* )
  F_DST=true
  DST=${PRINTER:=gwbb0}
  ;;

esac

if [ -z "${BIBLIOGRAPHY}" ] ; then

  case $MACH in

  hodi* | hosb* | disco | octet | boole )
    BIBLIOGRAPHY=/proj/starbase/tools/lib/papers/asi
    ;;

  mt* )
    BIBLIOGRAPHY=/mt/mtgzfs8/hw/tools/lib/papers/disco
    ;;

  esac

fi


# end of configurable defaults


# OK, we are ready to start the main program

: ${DWBHOME:=/usr/add-on/dwb}
: ${LOCAL:=/usr/add-on/local}
export DWBHOME LOCAL


prepend() {
  cat $@
  cat
}

if [ ${ARCH} = 'att' -a -d /usr/sbin -a "${F_POSTLOCAL}" = true ] ; then

  echo ${PATH} | fgrep '/usr/lib/lp/postscript' > /dev/null
  if [ $? -ne 0 ] ; then

    PATH=${PATH}:/usr/lib/lp/postscript

  fi
  F_POSTNEW=true

fi

if [ ${ARCH} != "att" -a -d /usr/5bin ] ; then

  PATH=/usr/5bin:${PATH}

fi

if [ -d ${DWBHOME} ] ; then

  F_DWBHOME=true
  case "${PATH}" in

  *${DWBHOME}/bin* )
    ;;

  * )
    PATH=${DWBHOME}/bin:${PATH}
    ;;

  esac

fi

if [ -n "${UNISON}" -a -d "${UNISON}" ] ; then

  U_PRTFMT=${UNISON}/bin/prtfmt
  U_PRT=${UNISON}/bin/prt

  if [ -x "${U_PRTFMT}" -a -x "${U_PRT}" ] ; then

    PRTFMT=$U_PRTFMT

  else

    UNISON=""

  fi

else

  UNISON=""

fi

if [ $F_DST = false ] ; then

  if [ -n "${PRINTER}" ] ; then

    DST=${PRINTER}
    F_DST=true

  fi

fi

FMTOPTS=""
PRTOPTS=""
DPOPTS=""

SIDES=2
FORM=nohole
FILES=""
FOPTS=""
PMODE="port"

F_DEBUG=false
F_INPUT=false
F_GETOPT=true
F_FALL=false

#F_DST=false				# already set above
F_COPIES=false
F_DEVTYPE=false
F_ZMODE=false
F_V=false
F_MAIL=false
F_PMODE=false
F_SIDES=false
F_FORM=false
F_QUIET=false
F_FORMATTER=false
F_HELP=false

S=files
OS=""
for A in "$@" ; do

#  echo "ARG ${A}" >&2

  if [ $F_GETOPT = true ] ; then

  case $A in

  '-D' )
    F_DEBUG=true
    ;;

  '-F' )
    OS=${S}
    S=formatter
    F_GETOPT=false
    ;;

  '-T' )
    OS=${S}
    S=devtype
    F_GETOPT=false
    ;;

  '-Z' )
    F_ZMODE=true
    ;;

  '-c' )
    OS=${S}
    S=copies
    F_GETOPT=false
    ;;

  '-d' )
    OS=${S}
    S=dst
    F_GETOPT=false
    ;;

  '-f' )
    OS=${S}
    S=form
    F_GETOPT=false
    ;;

  '-h' )
    F_HELP=true
    ;;

  '-m' )
    F_MAIL=true
    ;;

  '-p' )
    OS=${S}
    S=pmode
    F_GETOPT=false
    ;;

  '-q' )
    F_QUIET=true
    ;;

  '-s' )
    OS=${S}
    S=sides
    F_GETOPT=false
    ;;

  '-v' )
    F_V=true
    ;;

  '-' )
    F_INPUT=true
    ;;

  '-'* )
    echo "${P}: unknown option encountered ; use UNISON version" >&2
    exit 1
    ;;

  * )
    F_FALL=true
    ;;

  esac

  else

    F_FALL=true

  fi

  if [ $F_FALL = true ] ; then

    F_FALL=false
    case $S in

    files )
      FILES="${FILES} ${A}"
      ;;

    devtype )
      S=${OS}
      F_DEVTYPE=true
      DEVTYPE=${A}
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

    copies )
      S=${OS}
      COPIES=${A}
      F_COPIES=true
      ;;

    form )
      S=${OS}
      FORM=${A}
      F_FORM=true
      ;;

    formatter )
      S=${OS}
      F_FORMATTER=true
      FOPTS=${A}
      ;;

    pmode )
      S=${OS}
      PMODE=${A}
      F_PMODE=true
      ;;

    sides )
      SIDES=${A}
      F_SIDES=true
      S=${OS}
      ;;

    esac

    F_GETOPT=true

  fi

done

if [ $F_HELP = true ] ; then

  echo "${P}: see manual pages" >&2
  exit 1

fi

#if [ $F_DEBUG = true ] ; then set -x ; fi


if [ -z "${FILES}" ] ; then F_INPUT=true ; fi

if [ $F_V = true ] ; then

  FMTOPTS="${FMTOPTS} -v"

fi

if [ $F_SIDES = true ] ; then

  if [ "${SIDES}" -gt 2 -o "${SIDES}" -lt 0 ] ; then SIDES=2 ; fi

  PRTOPTS="${PRTOPTS} -s ${SIDES}"

fi

if [ $F_COPIES = true ] ; then

  PRTOPTS="${PRTOPTS} -c ${COPIES}"

fi

if [ $F_FORM = true ] ; then

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

  PRTOPTS="${PRTOPTS} -f ${FORM}"

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
  echo "${P}: unknown print mode \"${PMODE}\"" >&2
  exit 1
  ;;

esac

if [ $F_QUIET = true ] ; then

  PRTOPTS="${PRTOPTS} -q"

fi


FORMATTER=troff
if [ $F_FORMATTER = true ] ; then

  echo "${FOPTS}" | read O J 

  case "${O}" in

  [a-zA-Z]* )
    FORMATTER=${O}
    FOPTS=${J}
    ;;

  esac

fi

# end of command line option pre-processing


# establish program specific flags

HANDLE=default
DSTOPT=""
if [ $F_DST = true ] ; then

  case $DST in

  lp* )
    HANDLE=lp
    : ${DEVTYPE:=nroff}
    ;;

  gwbb* | di* )
    HANDLE=disco
    : ${DEVTYPE:=post}
    ;;

  null )
    HANDLE=null
    : ${DEVTYPE:=null}
    ;;

# assume a PostScript printer for all others
  * )
    : ${DEVTYPE:=post}
    ;;

  esac

  DSTOPT="-d ${DST}"

fi


case $HANDLE in

default | disco )
  if [ $F_DEVTYPE = true ] ; then

    FMTOPTS="${FMTOPTS} -T ${DEVTYPE}"

  fi
  ;;

esac


if [ $F_ZMODE = true ] ; then

  if [ -n "${UNISON}" ] ; then

    if [ $F_INPUT = true ] ; then

      $U_PRTFMT -Z ${FMTOPTS}

    else

      for F in $FILES ; do

        $U_PRTFMT -Z ${FMTOPTS} $F

      done

    fi

    exit 0

  fi

fi


# OK, we have finally decided to do some real printing here !!

TF1=/tmp/pfa${$}
TF2=/tmp/pfb${$}

cleanup() {
  rm -f $TF1 $TF2
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

RS=0
  if [ ${F_PMODE} = true ] ; then

    PRTOPTS="${PRTOPTS} -p ${PMODE}"
    add_dpopts -p ${PMODE}

  fi

  if [ ${F_INPUT} = true ] ; then

      cat > ${TF1}
      FILES=${TF1}

  fi

      if [ "${F_DWBHOME}" != true ] ; then

        echo "${P}: no DWBHOME package configured on this machine" >&2
        exit 1

      fi

# handle the case of no local UNISON package (we do it all ourselves)

      SAVE_DPOPTS=${DPOPTS}

      for F in ${FILES} ; do

        DPOPTS=${SAVE_DPOPTS}
        XOFFSET=""
        YOFFSET=""
        F_BOUNDING=false

        if [ ! -r $F ] ; then

          echo "${P}: file \"${F}\" not readable" >&2

        else

          F_PIC=false
          F_MS=false
          F_PICTURES=false
	  F_COLOR=false
	  F_PS=false

          PREPEND=""
          FILEOPT=$TF2
          CMD=""

          soelim $F > $TF2

	  AWK_PROG='/^\.[A-Za-z]|^\.\[/ { print substr($1,2,3) }'
          MACS=` awk "${AWK_PROG}" $TF2 2> /dev/null | sort -u `

# eliminate the newlines

          MACS=` echo ${MACS} `
#	echo macs are $MACS >&2

# properly separate all fields uniformly including the first and last

          MACS=" ${MACS} "
#	echo $MACS > macs
	  if [ $F_DEBUG = true ] ; then

	    echo "${MACS}" >&2

	  fi

# conditionally call in order 'tag', 'grap', 'gc2pic', 'pic', 'tbl', 'eqn'

	  case "${MACS}" in

          *so* )
            CMD="${CMD} soelim ${FILEOPT} |"
            FILEOPT=""
            ;;

          esac

	  case "${MACS}" in

          *'[ '* )
            RO=""
            fgrep '$LIST$' $TF2 > /dev/null
            if [ $? -eq 0 ] ; then RO="-e" ; fi

		if [ -s "${BIBLIOGRAPHY}" ] ; then

	    CMD="${CMD} refer ${RO} -p ${BIBLIOGRAPHY} ${FILEOPT} |"

		else

	    CMD="${CMD} refer ${RO} ${FILEOPT} |"

		fi

            FILEOPT=""
            ;;

	  esac

	  case "${MACS}" in

	  *IBR* | *BK* )
		if [ -s "${BIBLIOGRAPHY}" ] ; then

	    CMD="${CMD} referm -p ${BIBLIOGRAPHY} ${FILEOPT} |"

		else

	    CMD="${CMD} referm ${FILEOPT} |"

		fi

            FILEOPT=""
	    ;;

	  esac

	  case "${MACS}" in

	  *Ii* | *BG* )
	    CMD="${CMD} incima ${FILEOPT} |"
            FILEOPT=""
            F_PICTURES=true
	    ;;

	  esac

	  case "${MACS}" in

	  *vS* )
	    if [ -n "${FILEOPT}" ] ; then

              CMD="vgrind -f -w < ${FILEOPT} |"
              FILEOPT=""

	    else

              CMD="${CMD} vgrind -f -w |"

	    fi
            ;;

	  esac

	  case "${MACS}" in

          *TA* )
            CMD="${CMD} tag ${FILEOPT} |"
            FILEOPT=""
            ;;

          esac

	  case "${MACS}" in

          *G1* )
            CMD="${CMD} grap ${FILEOPT} |"
            F_PIC=true
            FILEOPT=""
            ;;

          esac

	  case "${MACS}" in

          *GS* )
            CMD="${CMD} gc2pic ${FILEOPT} |"
            F_PIC=true
            FILEOPT=""
            ;;

          esac

          if [ ${F_PIC} = true ] ; then

            CMD="${CMD} pic ${FILEOPT} |"
            FILEOPT=""

          else

	    case "${MACS}" in

            *PS* )
              CMD="${CMD} pic ${FILEOPT} |"
              FILEOPT=""
              ;;

            esac

          fi

	  case "${MACS}" in

          *TS* )
            CMD="${CMD} tbl ${FILEOPT} |"
            FILEOPT=""
            ;;

          esac

	  case "${MACS}" in

          *EQ* )
            CMD="${CMD} eqn ${FILEOPT} |" 
            FILEOPT=""
            ;;

          esac

          ROFFOPTS=""

	  case "${MACS}" in

          *VG* )
            ROFFOPTS="-mview"
            ;;

          *PP*T[PH]* | *SH*T[PH]* )
            ROFFOPTS="-man"
            ;;

          *V[SwhWH]* | *S[whW]* )
            ROFFOPTS="-mv"
            ;;

          *SH*' T '* )
            ROFFOPTS="-mv"
            ;;

          *' A '*SH* | *' C '*SH* | *' D '*SH* )
            ROFFOPTS="-mv"
            ;;

          *dN* | *f[CD]* | *wP* )
            ROFFOPTS="-mcs"
            ;;

          *AF* | *' H '* | *MT* | *' P '* | *S[KP]* | *[ABVD]L* )
            ROFFOPTS="-mm"
            ;;

          *P[FH]* | *' S '* )
            ROFFOPTS="-mm"
            ;;

          *[RLP]P* | *[TI]M* | *TL* | *[SN]H* | *M[RF]* | *EG* | *LT* )
            ROFFOPTS="-ms"
            ;;

          *E[FH]* )
            ROFFOPTS="-ms"
            ;;

          *[DF]S* )
            ROFFOPTS="-mm"
            ;;

          *IP* )
            ROFFOPTS="-ms"
            F_MS=true
            ;;

          *' B '*SH* )
            ROFFOPTS="-mv"
            ;;

          *[pil]p* )
            ROFFOPTS="-me"
            ;;

# I think that the following may have to always be last
	  *' P '* | *' R '* | *' B '* | *' I '* )
            ROFFOPTS="-mm"
	    ;;

          esac

	  if [ $F_MS = true ] ; then

	    case "${MACS}" in

	    *FL* | *FC* | *KF* | *P[123]* | *SP* | *Tm* | *' X '* )
	      ROFFOPTS="${ROFFOPTS} -mpm"
              ;;

	    esac

	  fi

	  case "${MACS}" in

          *BP* | *PI* )
            F_PICTURES=true
            ;;

	  esac

	  case "${MACS}" in

          *CL* )
            F_COLOR=true
            ;;

	  esac

	  case "${MACS}" in

	  *lM* | pM* )
	    ROFFOPTS="${ROFFOPTS} -mps"
            F_PS=true
            ;;
 
	  esac

          if [ $F_PS = false -a $F_PICTURES = true ] ; then

	    ROFFOPTS="${ROFFOPTS} -mpictures"
# the following causes problems with shifting of the printed image
#            F_BOUNDING=true

          fi

          if [ $F_PS = false -a $F_COLOR = true ] ; then

	    ROFFOPTS="${ROFFOPTS} -mcolor"

          fi

# permuted index macros

	  case "${MACS}" in

	  *xx* )
	    ROFFOPTS="${ROFFOPTS} -mptx"
	    ;;

	  esac

# other macro packages that should be prepended to the document

# the old "indent" command seems to be gone !
#
#	  case "${MACS}" in
#
#	  *HD* | *Fn* | *Pr* | *De* | *Du* )
#	     TMAC_INDENT=/usr/share/lib/tmac/tmac.indent
#	     if [ -r $TMAC_INDENT ] ; then
#
#              PREPEND="${PREPEND} /usr/share/lib/tmac/tmac.indent"
#
#	     fi
#            ;;
#
#	  esac
#

# fix up some problems based on the type of document that we know about

          case "${MACH}" in

          hocp* | nucleus | logicgate | nitrogen | mt* )
            case "${ROFFOPTS}" in

            *man* )
              XOFFSET="0.3"
              YOFFSET="-0.25"
              ;;

            *mm* | *ms* )
              case "${PMODE}" in

              port* )
                case "${DST}" in

                '*gz/'* | gz* | 2k* )
                  XOFFSET="0.15"
                  YOFFSET="-0.3"
                  ;;

		di* | hp* | ip* )
                  XOFFSET="0.30"
                  ;;

                esac
                ;;

              esac
              ;;

            esac
            ;;

          * )
            case "${ROFFOPTS}" in

            *man )
              XOFFSET="0.3"
              YOFFSET="-0.25"
              ;;

            esac
            ;;

          esac

          if [ "${F_BOUNDING}" = true ] ; then

            DPOPTS="${DPOPTS} -B"

          fi

          if [ -n "${XOFFSET}" ] ; then

            add_dpopts -x $XOFFSET

          fi

          if [ -n "${YOFFSET}" ] ; then

            add_dpopts -y $YOFFSET

          fi

# do we have some macro package to prepend to the file ?

	  if [ -n "${PREPEND}" ] ; then

	    if [ -n "${FILEOPT}" ] ; then

              CMD="prepend ${PREPEND} < ${FILEOPT} |" 
              FILEOPT=""

	    else

              CMD="${CMD} prepend ${PREPEND} |" 

	    fi

	  fi	
  
# OK, put the line with the formatter in place together
 
          CMD="${CMD} ${FORMATTER} ${ROFFOPTS} ${FOPTS} ${FILEOPT}"

# OK, prepare final form and go !

          if [ $F_ZMODE = false ] ; then

            CMD="${CMD} | ${DPOST} ${DPOPTS}"
            CMD="${CMD} | ${PRT} -d ${DST} -l postscript ${PRTOPTS}"
#            CMD="${CMD} | ${PRT} -d ${DST} -l troff ${PRTOPTS}"

          fi

          if [ $F_DEBUG = true -o $F_V = true ] ; then

            echo $CMD >&2

          else :

            eval $CMD

          fi

        fi

      done


cleanup
exit $RS



