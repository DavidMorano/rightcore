#!/usr/extra/bin/ksh
# SKYPEALERT


: ${LOCAL:=/usr/add-on/local}
: ${TOOLS:=/usr/add-on/exptools}
export LOCAL TOOLS


FPATH=${HOME}/fbin
if [[ ! -d "${FPATH}" ]] ; then
  FPATH=${LOCAL}/fbin
fi
export FPATH

pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib


U=dam
if [[ -n "${1}" ]] ; then
  U=${1}
fi

typeset -A DOW

DOW[0]="Sun"
DOW[1]="Mon"
DOW[2]="Tue"
DOW[3]="Wed"
DOW[4]="Thu"
DOW[5]="Fri"
DOW[6]="Sat"

builtin date 2> /dev/null

N=/dev/null
if username -q ${U} ; then

  TSW=$( date '+%w' )
#  TSD=$( date '+%y-%m-%d' )
#  TST=$( date %H%M:%S' )
  WW=${DOW[${TSW}]}
  
  DATE=$( date '+%y-%m-%d %H%M:%S' )
  print "A Skype-alert has been sent"
  print "at ${WW} ${DATE}."

  print "${DATE} skype call imminent" | notice ${U} -r -5 &

  {
    print "from: local (Skype Watcher)"
    print "subject: skype alert ${DATE}"
    print "to: ${U}"
    print
  } | mail $U

  if haveprogram tts ; then
    integer i n=3
    for (( i = 0 ; i < n ; i += 1 )) {
      print -- "skype alert" | tts -x
    } &
  fi

else
  print "Your recipient is not skype-able."
fi



