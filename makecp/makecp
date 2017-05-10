#!/usr/bin/ksh
# MAKECP


: ${GNU:=/usr/add-on/gnu}
export GNU

DN=/dev/null

if [[ $# -lt 2 ]] ; then
  print -u2 "${0}: not enough arguments"
  exit 1
fi

${GNU}/bin/cp --preserve=timestamps,mode "${@}"



