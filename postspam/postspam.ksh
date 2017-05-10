#!/usr/extra/bin/ksh
# POSTSPAM


: ${POSTFIX:=/usr/postfix}
: ${LOCAL:=/usr/add-on/local}
: ${PCS:=/usr/add-on/pcs}
: ${EXTRA:=/usr/extra}
export POSTFIX LOCAL PCS EXTRA

PRS=" ${POSTFIX} ${LOCAL} ${PCS} ${EXTRA}"

if [[ "${FPATH:0:1}" == ":" ]] ; then
  FPATH=${FPATH:1:200}
fi

for PR in ${PRS} ; do
  if [[ -d ${PR} ]] ; then
    FBIN=${PR}/fbin
    if [[ -d ${FBIN} ]] ; then
      if [[ -n "${FPATH}" ]] ; then
        FPATH="${FPATH}:${FBIN}"
      else
        FPATH="${FBIN}"
      fi
    fi
  fi
done
export FPATH

for PR in ${PRS} ; do
  pathadd PATH ${PR}/bin
  pathadd PATH ${PR}/sbin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done
export PATH LD_LIBRARY_PATH

if whence -q builtin ; then
  BILIB=${LOCAL}/lib/liblkcmd.so
  if [[ -x ${BILIB} ]] ; then
    builtin -f ${BILIB} pt logfile
  fi
fi

PN=postspam
BF=bogofilter
PV=0
TF=/tmp/pd${$}

DN=/dev/null
LOGFILE=${POSTFIX}/log/${PN}


FILES=

RF_DEBUG=false
RF_UNDO=false

S=files
OS=""
for A in "${@}" ; do
  case ${A} in
  '-D' )
    RF_DEBUG=true
    ;;
  '-u' )
    RF_UNDO=true
    ;;
  * )
    case ${S} in
    files )
      FILES="${FILES} ${A}"
      ;;
    esac
    ;;
  esac
done


OPTS=
if ${RF_UNDO} ; then
  OPTS="-S"
else
  OPTS="-s"
fi


logfile -c ${LOGFILE} -n ${PN}:${PV} -s 10m |&

function logprint {
  typeset V="${1}"
  if [[ -n "${V}" ]] ; then
    print -p -- "${V}"
  fi
}

function cleanup {
  rm -f ${TF}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

EX=0

CF=${POSTFIX}/etc/${BF}.cf
BOGOFILTER_DIR=${POSTFIX}/var/${BF}
export BOGOFILTER_DIR

  if pt -eq ${BF} && [[ -d ${BOGOFILTER_DIR} ]] ; then
    logprint "bogofilter"
    ${BF} -c ${CF} -d ${BOGOFILTER_DIR} ${OPTS}
    EX=$?
    logprint "bogofilter ex=${EX}"
  fi

    logprint "%T exiting ex=${EX}"
cleanup


