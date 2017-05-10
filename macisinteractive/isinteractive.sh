#!/usr/bin/ksh
# ISINTERACTIVE

EX=1
if tty > /dev/null ; then
  EX=0
fi
return ${EX}

