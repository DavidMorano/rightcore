#!/bin/ksh
# elfdump-instr


# 00/06/14, Dave Morano
# This program was written to prepare for Levo instruction decode testing.



P=`basename ${0} `

if [ $# -lt 1 ] ; then
  echo "${P}: no file specified" >&2
  exit 1
fi

if [ ! -r "${1}" ] ; then
  echo "${P}: file \"${1}\" was not found" >&2
  exit 1
fi


TF1=/tmp/edi${RANDOM}
TF2=/tmp/edi${RANDOM}

cleanup() {
  rm -f $TF1 $TF2
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

/usr/ccs/bin/dump -h $1 | fgrep '.text' > $TF1

if [ ! -s "${TF1}" ] ; then
  echo "${P}: there were no instructions in this object file" >&2
  exit 1
fi

SNO=$( line < $TF1 | cut -d '[' -f 2 | cut -d ']' -f 1 )

/usr/ccs/bin/dump -s -d ${SNO} $1 | fgrep -v ':' > ${TF2}

${PCS}/bin/dehex ${TF2}

cleanup


