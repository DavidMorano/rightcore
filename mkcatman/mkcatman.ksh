#!/usr/extra/bin/ksh
# MKCATMAN

#
# Symopsis:
# $ mkcatman 1 2 3 4 5 7d 3socket 3xnet 
#
#

: ${CATMAN:=/usr/add-on/catman}
export CATMAN

MANDNAME=/usr/man
PREFIX=cat

PN=${0##*/}

if [[ -d ${CATMAN} ]] ; then
  for A in "${@}" ; do
    if [[ -n "${A}" ]] ; then
      CATDNAME=${MANDNAME}/${PREFIX}${A}
      if [[ ! -d ${CATDNAME} ]] ; then
        rm -fr ${CATDNAME}
        CALT=${CATMAN}/${PREFIX}${A}
        if [[ ! -d ${CALT} ]] ; then mkdir -m 0777 ${CALT} ; fi
        ln -s ${CALT} ${CATDNAME}
      fi
    fi
  done
else
  print -u2 -- "${PN}: CATMAN directory inaccessible"
fi


