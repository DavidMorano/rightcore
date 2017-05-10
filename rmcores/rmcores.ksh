#!/usr/extra/bin/ksh
# RMCORES

DN=/dev/null
if [[ $# -gt 0 ]] ; then
  for D in $* ; do
    if [[ -d "${D}" ]] ; then
      filerm -cores ${D}
    fi
  done 2> ${DN}
else
  D=${PWD}
  if [[ -d "${D}" ]] ; then
    filerm -cores ${PWD}
  fi
fi


