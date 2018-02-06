#!/usr/extra/bin/ksh
# TESTINDEX1


: ${LOCAL:=/usr/add-on/local}
: ${AST:=/usr/add-on/ast}
: ${NCMP:=/usr/add-on/ncmp}
: ${GNU:=/usr/add-on/gnu}
: ${EXTRA:=/usr/extra}
export LOCAL AST NCMP GNU EXTRA

: ${DOCUMENTS:=/usr/add-on/documents}
: ${ARTICLES:=/usr/add-on/articles}
: ${REPORTS:=/usr/add-on/reports}
: ${INTERVIEWS:=/usr/add-on/interviews}
export DOCUMENTS ARTICLES REPORTS INTERVIEWS

PRS=" ${HOME} ${LOCAL} ${EXTRA} "

if [[ "${FPATH:0:1}" == ":" ]] ; then
  FPATH=${FPATH:1:200}
fi

for PR in ${PRS} ; do
  B=${PR}/fbin
  if [[ -d ${B} ]] ; then
    if [[ -n "${FPATH}" ]] ; then
      FPATH="${FPATH}:${B}"
    else
      FPATH=${B}
    fi
  fi
done
export FPATH

for PR in ${PRS} ; do
  pathadd PATH ${PR}/bin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done


EXPAT=${TMPDIR:=/tmp}/expat${$}

function cleanup {
  rm -f ${EXPAT}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

{
  print AppleDouble
  print .swd
  print .deb
} > ${EXPAT}


IDIR=${LOCAL}/var/txtindexes
SUFS=msg,txt,mm,ms,tex,dwb,bib
FOPTS="-ia"
TABLEN=4000


SRCDIR=${HOME}/src/mkindexing


BASEDIR=${DOCUMENTS}
INAME=documents
#print -u2 -- "${0}: b=${BASEDIR} n=${INAME}"

MKINDEXING_DB=${IDIR}/${INAME}
export MKINDEXING_DB
if [[ -d ${BASEDIR} ]] && cd ${BASEDIR} ; then
  {
    ls | filefind -af - -s ${SUFS} ${FOPTS} | fgrep -v AppleDouble
  } | mkkey -Q -w -af - > ${SRCDIR}/oo.keys
fi


