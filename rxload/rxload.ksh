#!/bin/ksh
# RXLOAD (Remote XLOAD)


if [ -d /usr/sbin ] ; then
  MACH=$( uname -n )
else
  MACH=$( hostname )
fi

if [ -n "${1}" ] ; then
  TARGET=$1
  shift
else
  TARGET=rca
fi


: ${LOCAL:=/usr/add-on/local}
: ${PCS:=/usr/add-on/pcs}
export LOCAL PCS


if [ -z "${RXPORT}" ] ; then
  RXPORT=DISPLAY
else
  echo ${RXPORT} | fgrep DISPLAY > /dev/null
  if [ $? -ne 0 ] ; then
    RXPORT="${RXPORT},DISPLAY"
  fi
fi

#export RXPORT
unset RXPORT


REX_OPTS="-x"
case $TARGET in

rc* )
  ;;

mt* )
  REX_OPTS="${REX_OPTS} -e ${LOCAL}/lib/rshe/environment"
  ;;

hocp* )
  ;;

* )
  REX_OPTS="${REX_OPTS} -e .profile"
  ;;

esac


XLOAD_OPTS="-geometry 114x71 -update 1 -hl white "


cex $TARGET $REX_OPTS ksh <<\-EOF 2> /dev/null
	cd /tmp
	DEVNULL=/dev/null
	xload $XLOAD_OPTS < $DEVNULL > $DEVNULL 2>&1 &
EOF



