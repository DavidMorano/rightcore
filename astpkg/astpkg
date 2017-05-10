#!/usr/bin/ksh
# ASTPKG


PACKAGEROOT=/sb07/astpkg
export PACKAGEROOT

AST=${PACKAGEROOT}/arch/sol8.sun4
export AST


export PATH=${AST}/bin:${PATH}
export LD_LIBRARY_PATH=${AST}/lib:${LD_LIBRARY_PATH}
export MANPATH=${AST}/man:${MANPATH}


export PS1="astpkg> "

exec ksh "${@}"



