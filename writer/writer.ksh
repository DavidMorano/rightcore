#!/usr/bin/ksh
# WRITER


: ${LOCAL:=/usr/add-on/local}
export LOCAL


FPATH=${HOME}/fbin
if [[ ! -d "${FPATH}" ]] ; then
  FPATH=${LOCAL}/fbin
fi
export FPATH


pathadd PATH ${HOME}/bin
pathadd LD_LIBRARY_PATH ${HOME}/lib

pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib


U=dam
if [[ -n "${1}" ]] ; then
  U=${1}
fi

N=/dev/null
if username -q ${U} ; then

  RECIPNAME=$( userinfo $U fullname )

  print -- "your recipient is> ${RECIPNAME}"
  print -- "You can start typing immediately but it may take a while for"
  print -- "a response from your recipient."
  print -- "Typing a control-D character on a line by itself ends the session."

  DATE=$( date '+%y-%m-%d %H%M:%S' )
  print "${DATE} write call is pending" | notice ${U} -r -3

  {
    print "from: local (Write Watcher)"
    print "subject: write alert ${DATE}"
    print "to: ${U}"
    print
  } | mail $U

  write $U

else
  print "Your recipient is not accessible."
fi



