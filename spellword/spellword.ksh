#!/usr/extra/bin/ksh
# SPELLWORD (local)


: ${HOME:=$( userhome )}
: ${LOCAL:=/usr/add-on/local}
export HOME LOCAL


PATH=${LOCAL}/bin:${PATH}
export PATH


if [ ! -d ${HOME}/lib ] ; then
  mkdir ${HOME}/lib
fi

if [ ! -d ${HOME}/lib/wwb ] ; then
  mkdir ${HOME}/lib/wwb
fi

for W in $* ; do
  print -- ${W}
done >> ${HOME}/lib/wwb/spelldict


