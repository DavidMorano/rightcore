#!/usr/extra/bin/ksh
# LFMT

: ${COLUMNS:=80}
: ${NCMP:=/usr/add-on/ncmp}
export COLUMNS NCMP

P_FMT=${NCMP}/bin/fmt


WIDTH=${COLUMNS}
FILES=
OPT_C=false
OPT_S=false

S=files
OS=""
for A in "${@}" ; do
  case ${A} in
  '-c' )
    OPT_C=true
    ;;
  '-s' )
    OPT_S=true
    ;;
  '-cs' )
    OPT_C=true
    OPT_S=true
    ;;
  '-sc' )
    OPT_C=true
    OPT_S=true
    ;;
  '-w' )
    OS=${S}
    S=width
    ;;
  '-'* )
    REM=${A:1}
    if [[ -n "${REM}" ]] ; then
      WIDTH=${REM}
    fi
    ;;

  * )
    case ${S} in
    width )
      WIDTH=${A}
      S=${OS}
      ;;
    files )
      FILES="${FILES} ${A}"
      ;;
    esac
    ;;
  esac
done

POPTS=
if [[ -n "${WIDTH}" ]] ; then
  POPTS="${POPTS} -w ${WIDTH}"
fi

${P_FMT} ${POPTS} ${FILES}


