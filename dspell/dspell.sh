#!/usr/extra/bin/ksh
# DSPELL


P=${0}

: ${TMPDIR:=/tmp}
: ${PROGRAMROOT:=${HOME}}

SCRATCH=${TMPDIR}/dspell${$}

if [ -z "$1" -o ! -f "$1" ] ; then
	echo "${P}: USAGE> dspell <file_name>" >&2
	exit 1
fi

if [ ! -d "${SCRATCH}" ]  ; then
	mkdir ${SCRATCH}
fi

#echo "copying " $1 "into ${SCRATCH}/spellin.tmp"
cp $1 ${SCRATCH}/spellin.tmp
spell $1 > ${SCRATCH}/spell.out
duplic < ${SCRATCH}/spell.out > ${SCRATCH}/spelldup.tmp
vi ${SCRATCH}/spelldup.tmp

INPUT=${SCRATCH}/spelldup.tmp
OUTPUT=${SCRATCH}/spelscp.tmp

if ${PROGRAMROOT}/bin/makescript < ${INPUT} > ${OUTPUT} ; then 

	cat ${SCRATCH}/spelscp.tmp | ex ${SCRATCH}/spellin.tmp 
	echo "result is in ${SCRATCH}/spellin.tmp"
	echo "ok to overwrite " $1 "[ok] ? "
	read response
	if test "${response}" = "ok"; then 
		echo "overwriting ..."
		mv ${SCRATCH}/spellin.tmp $1 
	else
		echo "output is still in ${SCRATCH}/spellin.tmp"
	fi
else
	/bin/rm -f ${SCRATCH}/spelldup.tmp ${SCRATCH}/spelscp.tmp 
	/bin/rm -f ${SCRATCH}/spellin.tmp
fi

rm -fr ${SCRATCH}



