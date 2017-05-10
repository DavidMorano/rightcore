#!/usr/bin/ksh
# MKARTICLE

#
# An alternative to using the 'linefold' program is to use
# the standard 'fold' program with 'fold -s' as an invocation.
#

P_FMT=fmt


TF=${TMPDIR:=/tmp}/mka${$}

function cleanup {
  rm -f ${TF}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

for F in "${@}" ; do
  if [ -r "${F}" ] ; then
    textclean -o mscrap,leading ${F} | linefold -i 0 | {
      ${P_FMT} | textclean -o leading | ${P_FMT} > $TF
    }
    mv ${TF} ${F}
  fi
done

cleanup


