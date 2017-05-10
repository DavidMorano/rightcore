#!/usr/bin/ksh
# SM (Send-Mail)


MM=${1}
if [[ -r "${MM}" ]] ; then
  imail < ${MM}
  msgclean -nice 0 -t 0 ${MM}
fi


