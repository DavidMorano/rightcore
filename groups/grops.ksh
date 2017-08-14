#!/usr/bin/ksh
# GROUPS


# = commentary
#
# Why is this needed?
#
# The program 'groups(1)' SHOULD be just:
#	$ id -Gn ${1}
#
# Why isn't it?
#
# = first reason:
# Because the stupid GNU version is messed up!
# It prints a stupid colon (':') character when a username is supplied
# (it doesn't print the colon when no usernames are supplied).
#
# = second reason
# There is disagreement over what the 'id(1)' program is even 
# supposed to do?  For example, the traditional Slowlaris version
# doesn't even support the 'u', 'g', 'G', or 'n' argument options!
# Note that the Slowlaris XPG4 version does work correctly, but
# we do not know if it is around all of the time.
#
#


U=${1}

if whence userinfo > /dev/null ; then
  if [[ -z "${U}" ]] ; then U="-" ; fi
  LIST=$( userinfo "${U}" group groups ) 
else
  P=/usr/xpg4/bin/id
  if [[ -x ${P} ]] ; then
    LIST=$( id -Gn "${U}" ) 
  else
    LIST=${P}
  fi
fi
print ${LIST}


