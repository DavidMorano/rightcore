#!/usr/bin/ksh
# POSTMAIL

export POSTFIX=/usr/postfix
export PATH=${PATH}:${POSTFIX}/bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${POSTFIX}/lib

${POSTFIX}/bin/postmail "${@}"


