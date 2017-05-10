#!/usr/bin/ksh
# AST


if [[ ! -d "${AST}" ]] ; then
  unset AST
fi

: ${AST:=/usr/add-on/ast}
export AST


export PATH=${AST}/bin:${PATH}
export LD_LIBRARY_PATH=${AST}/lib:${LD_LIBRARY_PATH}
export MANPATH=${AST}/man:${MANPATH}


# important thing since we are not totally "in" to this AST stuff!

COEXPORT=PATH,LD_LIBRARY_PATH,DISPLAY
export COEXPORT


PS1="ast> "
export PS1

if [[ -n "${1}" ]] ; then
  PROG=${1}
  shift
else
  PROG=ksh
fi

exec ${PROG} "${@}"


