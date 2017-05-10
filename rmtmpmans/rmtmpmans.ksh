#!/usr/extra/bin/ksh
# RMTMPMANS


DN=/dev/null
PN=${0##*/}
ND=2


TD=/tmp

{
  find ${TD} -type f -mtime +${ND} -name 'mp*' -print | {
    xargs rm -f
  } 
} 2> ${DN}


