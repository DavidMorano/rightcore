#!/bin/ksh
# XTERMINAL

#
#	Arguments:
#		machine		name of remote machine to login to
#


if [ $# -lt 1 -o -z "${1}" ] ; then
  exit 1
fi

if [ -d /usr/sbin ] ; then
  MACH=$( uname -n )
else
  MACH=$( hostname )
fi


RMACH=$1
: ${SHELL:=ksh}

W=telnet
F_SPECIAL=""

#
#case $MACH in
#
#hodi[a-z] )
#  on $RMACH 2> /dev/null echo YES | fgrep YES > /dev/null
#  if [ $? -eq 0 ] ; then W=on ; fi
#  ;;
#
#esac
#


if [ -n "${RMACH}" ] ; then

case ${MACH}:${RMACH} in

rc*:rc* )
  W=rex
  F_SPECIAL=yes
  ;;

rc*:*.coe.neu.edu | rc*:*.ece.neu.edu | rc*:*.ele.uri.edu )
  W=slogin
  ;;

* )
  W=telnet
  ;;

esac

case $W in

slogin )
  CMD="slogin ${RMACH}"
  ;;

rlogin )
  CMD="rlogin ${RMACH}"
  ;;

on )
  CMD="on -i ${RMACH} ${SHELL}"
  ;;

rex )
  CMD="rex -xd ${RMACH} /bin/ksh -i"
  ;;

rshe )
  CMD="rshe -xd $RMACH /bin/ksh -i"
  ;;

rx )
  CMD="rx ${RMACH} /bin/ksh -i"
  ;;

rl )
  CMD="rl ${RMACH}"
  ;;

* )
  CMD="telnet ${RMACH}"
  ;;

esac

fi


if [ -n "${F_SPECIAL}" ] ; then

  OPTS="-si -sl 1000"
#  exec rshe -dx -n $RMACH xterm $OPTS -name $1 -title $RMACH
  exec cex -n $RMACH xterm $OPTS -name $1 -title $RMACH

else

  if [ -n "${RMACH}" ] ; then

    case $W in

    telnet )
      exec rex -nx hocpw xterm -si -sl 1000 -name $1 -title $RMACH -e $CMD
      ;;

    slogin )
      exec xterm -si -sl 1000 -name $1 -title $RMACH -e $CMD
      ;;

    esac

  fi

fi



