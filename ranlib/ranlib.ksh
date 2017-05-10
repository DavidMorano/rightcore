#!/bin/ksh
# RANLIB


: ${AST:=/usr/add-on/ast}
: ${GNU:=/usr/add-on/gnu}
export AST GNU


PDIR=${0%/*}
A=${0##*/}
PNAME=${A%.*}



PROGS=
PROGS="${PROGS} /usr/bin/ranlib"
PROGS="${PROGS} /usr/ccs/bin/ranlib"
PROGS="${PROGS} ${GNU}/bin/ranlib"


for PF in $PROGS ; do

  if [[ -x $PF ]] ; then
    break
  fi

done

if [ ! -x $PF ] ; then

  echo "${0}: could not find underlying mailer program" >&2
  exit 1

fi

execname $PF $PNAME "${@}"




