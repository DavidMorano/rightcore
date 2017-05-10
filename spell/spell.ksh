#!/usr/extra/bin/ksh
# SPELL (NCMP)


: ${LOCAL:=/usr/add-on/local}
: ${PREROOT:=/usr/preroot}
: ${NCMP:=/usr/add-on/ncmp}
export LOCAL PREROOT NCMP

PRS=" ${HOME} ${LOCAL} ${PREROOT} ${EXTRA} "

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
  pathadd LD_LIBRARY_PATH ${PR}/lib
done

: ${HOME:=$( userhome )}
P_SPELL=${PREROOT}/bin/spell 

# start of psuedo regular non-changeable program

TFA=/tmp/spella${$}
TFB=/tmp/spellb${$}
TFC=/tmp/spellc${$}

cleanup() {
  rm -f ${TFA} ${TFB} ${TFC}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


FILES=
ARGS=
LF=

RF_DEBUG=false
RF_INPUT=false

S=files
OS=
for A in "${@}" ; do
  case ${A} in
  '-D' )
    RF_DEBUG=true
    ;;
  '-'[bvx] )
    ARGS="${ARGS} ${A}"
    ;;
  '-' )
    RF_INPUT=true
    ;;
  '+'* )
    LF=$( echo ${A} | cut -c 2-256 )
    RF_LOCALFILE=true
    ;;
  '-'* )
    print -u2 -- "${P}: unknown option \"${A}\""
    exit 1
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

UNIX_WORDS=/var/spell/local.words
SPELLDICT_WORDS=${HOME}/lib/wwb/spelldict
USER_WORDS=${HOME}/share/dict/user.words
ISPELL_WORDS=${HOME}/.ispell_english

EXTRAS=" ${UNIX_WORDS} ${SPELLDICT_WORDS} ${USER_WORDS} ${ISPELL_WORDS}"

for E in ${EXTRAS} ; do
  if [[ -s "${E}" ]] ; then
    cat ${E} >> ${TFA}
  fi
done

CMD_WORDS=${LF}
if [[ -s "${CMD_WORDS}" ]] ; then
  cat ${CMD_WORDS} >> ${TFA}
fi

# if we have some, merge them all together

if [[ -s "${TFA}" ]] ; then
  sort -u ${TFA} | sort -d -f > ${TFB}
  ARGS="+${TFB} ${ARGS}"
fi

# go for it

execname ${P_SPELL} ${0} ${ARGS} ${FILES}

cleanup


