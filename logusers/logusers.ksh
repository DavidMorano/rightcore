#!/usr/bin/ksh
# LOGUSERS


: ${LOCAL:=/usr/add-on/local}
export LOCAL


FPATH=${LOCAL}/fbin
export FPATH


autoload logusers

if pt -f -q ; then
  logusers "${@}"
fi



