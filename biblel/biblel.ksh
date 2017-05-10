#!/usr/bin/ksh
# BIBLEL


: ${LOCAL:=/usr/add-on/local}
export LOCAL

WORDS=${LOCAL}/share/dict/bible.words

if [[ -r "${WORDS}" ]] ; then
  for W in "${@}" ; do
    if [[ -n "${W}" ]] ; then
      look -f -d "${W}" ${WORDS}
    fi
  done
else
  print -u2 "${0}: database not found"
fi



