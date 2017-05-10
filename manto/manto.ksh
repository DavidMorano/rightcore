#!/usr/bin/ksh
# MANTO - prints path to manual page (via searching MANPATH)
# 
#

P=manto


USAGE="USAGE> manto <manual_page>"

if [ $# -lt 1 ] ; then

  echo "${P}: argument required" >&2
  echo "${P}: ${USAGE}" >&2
  exit 1

fi

if [ -z "${1}" -o -z "${MANPATH}" ] ; then 
  exit 0
fi

CWD=${PWD:-$( pwd )}

for E in "${@}" ; do

  IFS=:
  for I in ${MANPATH} ; do

    if [ -z "${I}" ] ; then

      for M in *man*/${E}.[0-9]* ; do

        if [ -f ${M} ] ; then
          echo ${CWD}/${M}
        fi

      done

    else

      for M in ${I}/*man*/${E}.[0-9]* ; do

        if [ -f ${M} ] ; then
          echo $M
        fi

      done

    fi

  done

done


