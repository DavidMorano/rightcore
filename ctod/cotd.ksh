#!/usr/bin/ksh
# COTD


: ${EXTRA:=/usr/extra}
: ${LOCAL:=/usr/add-on/local}
export EXTRA LOCAL


FPATH=${LOCAL}/fbin
if [[ ! -d "${FPATH}" ]] ; then
  FPATH=${EXTRA}/fbin
fi
export FPATH


if whence pathadd > /dev/null ; then

  pathadd PATH ${LOCAL}/bin
  pathadd LD_LIBRARY_PATH ${LOCAL}/lib

  pathadd PATH ${EXTRA}/bin
  pathadd LD_LIBRARY_PATH ${EXTRA}/lib


  if whence commandment > /dev/null ; then
    N=$( date '+%d' )
    integer n
    (( n = ( ${N} % 10 ) ))
    commandment ${n}
  fi

fi



