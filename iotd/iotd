#!/usr/extra/bin/ksh
# IOTD (Issue of the Day)


ISSUE=/etc/issue
LIBLKCMD=liblkcmd.so


: ${LOCAL:=/usr/add-on/local}
export LOCAL

builtin -f ${LOCAL}/lib/${LIBLKCMD} 2> /dev/null


  if [[ -r ${ISSUE} ]] ; then
    shcat ${ISSUE}
  fi



