#!/usr/bin/ksh
# SHGREP


N=/dev/null

P_FGREP=fgrep
if whence ${P_FGREP} > $N ; then :
else
  P_FGREP=/usr/bin/fgrep
fi

exec ${P_FGREP} "${@}"



