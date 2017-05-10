#!/usr/bin/ksh
# FINGERBIBLEQ


: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
export LOCAL NCMP


PATH=${LOCAL}/bin:${PATH}
LD_LIBRARY_PATH=${LOCAL}/bin:${LD_LIBRARY_PATH}
export PATH LD_LIBRARY_PATH


A="${1}"
WORDS=$( fieldwords "${A}" )

#print -u2 WORDS=${WORDS}
bibleq -o indent,book,interactive ${WORDS}



