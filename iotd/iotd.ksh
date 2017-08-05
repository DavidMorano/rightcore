#!/usr/extra/bin/ksh
# IOTD (Issue of the Day)


ISSUE=/etc/issue
LIBLKCMD=liblkcmd.so


: ${LOCAL:=/usr/add-on/local}
export LOCAL

LIB=${LOCAL}/lib/${LIBLKCMD}
DN=/dev/null

builtin -f ${LIB} 2> ${DN}


  if [[ -r ${ISSUE} ]] ; then
    shcat ${ISSUE}
  fi


