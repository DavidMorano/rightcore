#!/usr/bin/ksh
# FINGERQUOTE


: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
export LOCAL NCMP


PATH=${LOCAL}/bin:${PATH}
LD_LIBRARY_PATH=${LOCAL}/bin:${LD_LIBRARY_PATH}
export PATH LD_LIBRARY_PATH


A="${1}"
WORDS=$( fieldwords "${A}" )

quote ${WORDS}



