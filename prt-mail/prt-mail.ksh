#!/bin/ksh
# PRT-PCSMAIL


P=prt-mail


: ${OPENWINHOME:=/usr/openwin}
export OPENWINHOME

if [ ! -x ${OPENWINHOME}/bin/mp ] ; then

  echo "${P}: could not find OpenWindows" >&2
  exit 1

fi

PATH=${OPENWINHOME}/bin:${PATH}
export PATH


mktmp() {
  C=0
  TF=/tmp/pm${C}${$}
  while [ -r ${TF} ] ; do

    C=`expr $C + 1 `
    TF=/tmp/pm${C}${$}

  done
  echo $TF
}

TF=`mktmp`

cleanup() {
  rm -f $TF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


FILES=""
SIDES=2
COPIES=1
MODE=port
FORM=nohole

F_DEBUG=false
F_DST=false
F_SIDES=false
F_COPIES=false
F_MODE=false
F_FORM=false

if [ -n "${PRINTER}" -o -n "${LPDEST}" ] ; then

  DST=${PRINTER:-${LPDEST}}
  F_DST=true

fi

S=files
OS=""
for A in "${@}" ; do

  case $A in

  '-c' )
    OS=${S}
    S=copies
    ;;

  '-d' )
    OS=${S}
    S=dst
    ;;

  '-s' )
    OS=${S}
    S=sides
    ;;

  '-p' )
    OS=${S}
    S=mode
    ;;

  '-f' )
    OS=${S}
    S=form
    ;;

  '-D' )
    F_DEBUG=true
    ;;

  '-'* )
    echo "${0}: unknown option \"${A}\" " >&2
    exit 1
    ;;

  * )
    case $S in

    copies )
      COPIES=${A}
      F_COPIES=true
      S=${OS}
      ;;

    files )
      FILES="${FILES} ${A}"
      ;;

    dst )
      DST=${A}
      F_DST=true
      S=${OS}
      ;;

    sides )
      SIDES=${A}
      F_SIDES=true
      S=${OS}
      ;;

    mode )
      MODE=${A}
      F_MODE=true
      S=${OS}
      ;;

    form )
      FORM=${A}
      F_FORM=true
      S=${OS}
      ;;

    esac
    ;;

  esac

done

PRTOPTS=""
if [ $F_DST = true ] ; then
  PRTOPTS="${PRTOPTS} -d ${DST}"
fi

if [ $F_SIDES = true ] ; then
  PRTOPTS="${PRTOPTS} -s ${SIDES}"
fi

if [ $F_COPIES = true ] ; then
  PRTOPTS="${PRTOPTS} -c ${COPIES}"
fi


if [ -z "${FILES}" ] ; then
  cat > $TF
  FILES=$TF
fi

for F in ${FILES} ; do

  if [ -s $F ] ; then :

    if [ $F_DEBUG = true ] ; then
      echo "mp < ${F} -F | prt -l postscript ${PRTOPTS}" >&2
    else
      mp < $F -F | prt -l postscript $PRTOPTS &
    fi

  else
    echo "${P}: empty or nonexistent file\n" >&2
  fi

done

cleanup



