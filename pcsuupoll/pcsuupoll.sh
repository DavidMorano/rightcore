#!/bin/sh
# UUPOLL


: ${PCS:=/usr/add-on/pcs}
export PCS


PATH=${PCS}/bin:${PATH}
export PATH


P=`basename ${0} `

if [ $# -lt 1 ] ; then
  echo "${P}: a system must be specified" >&2
  exit 1
fi

for SYS in $* ; do
  pcsuux -r ${SYS}!
done

pcsuusched &



