#!/usr/bin/ksh
# SUBMITTER


SUBDIR=${HOME}/var/spool/submitd
SVC=sub


: ${PWD:=$( pwd )}


SF=$( pcsjobfile -r )
JOBID=$( basename $SF )

cleanup() {
  rm -f $SF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


{
  echo
  echo "cd ${PWD}"
  envget -export
  echo JOBID=${JOBID}
  echo export JOBID
  if [[ -z "${NAME}" ]] ; then
    echo "NAME='${NAME}'"
    export NAME
  fi
  if [[ -z "${FULLNAME}" ]] ; then
    echo "FULLNAME='${FULLNAME}'"
    export FULLNAME
  fi
  echo
} >> $SF


cat >> ${SF}

mv $SF ${SUBDIR}/${JOBID}.${SVC}
#cat $SF




