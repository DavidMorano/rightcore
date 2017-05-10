#!/usr/bin/ksh
# NODENAME


if [[ -z "${NODE}" ]] ; then
  A=$( uname -n )
  NODE=$(A%%.*}
fi

print -- ${NODE}



