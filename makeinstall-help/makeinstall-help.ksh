#!/usr/extra/bin/ksh
# MAKEINSTALL-HELP


: ${SRCROOT:=/usr/add-on/local}

HELPDIR=${1}
if [[ -z "${HELPDIR}" ]] ; then
    HELPDIR=${SRCROOT}/share/help
fi

for F in *.help ; do
  if [[ -r "${F}" ]] ; then
    makenewer -o rmsuf ${F} ${HELPDIR}
  fi
done


