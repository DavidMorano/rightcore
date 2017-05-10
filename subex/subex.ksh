#!/usr/bin/ksh
# SUBEX


: ${DOMAIN:=$( logname - domain )}
: ${NODE:=$( uname -n )}
: ${LOGNAME:=$( logname )}
: ${HOME:=$( logdir )}
export DOMAIN NODE LOGNAME HOME


MAILUSER=${LOGNAME}
FROMADDR="${LOGNAME}@${DOMAIN}"
SUBJECT="submit_job ${NODE}"


TFI=/tmp/subexi${$}
TFO=/tmp/subexo${$}
TFE=/tmp/subexe${$}

cleanup() {
  rm -f $TFI $TFO $TFE
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

#exec 2> /home/student/dmorano/rje/subex.err

#/bin/env >&2

cat > $TFI

if [[ -s $TFI ]] ; then

nice -4 ksh $TFI > $TFO 2> $TFE

{
  if [[ -s $TFO ]] ; then
#print -u2 STDOUT
    echo standard output
    cat ${TFO}
  fi
  if [[ -s $TFE ]] ; then
#print -u2 STDERR
    echo standard error
    cat ${TFE}
  fi
} | mkmsg -s "${SUBJECT}" -f "${FROMADDR}" $MAILUSER | mailbridge $MAILUSER

fi

cleanup



