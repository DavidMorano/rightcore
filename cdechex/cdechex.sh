#!/usr/bin/ksh
# CDECHEX


if [ -z "${1}" ] ; then
  exit 1
fi


V=$( echo "obase=16 ; ${1}" | bc )

VV=$( echo 000000${V} | cut


