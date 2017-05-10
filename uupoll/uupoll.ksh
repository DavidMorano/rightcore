#!/usr/bin/ksh
# UUPOLL.KSH


# Program Note:
#	This program must be executed as an 'exec(2)' type of
#	program using the 'exec(2)' escape characters at the
#	top of the file; namely, the characters '#!'.
#
#	Also, this program must be a Korn SHELL interpreter
#	program and not a Bourne shell, even though the language
#	in this program is Bourne language, because the Bourne
#	shell always resets the effective UID to the real UID
#	and this program MUST execute with an effective UID of
#	'uucp'.  This is why the program has two parts!!
#


UUROOT=${NCMP:=/abroad/morano/add-on/ncmp}

PATH=/usr/bin:/usr/lib/uucp
export PATH
UUSPOOL=${UUROOT}/var/spool/uucp

# this is the sub directory that the 'C.' file will be queued in
DEFQUEUE=Z

umask 022
set +e

for site in "${@}" ; do

  if [ ! -d ${UUSPOOL}/${site} ] ; then
    mkdir ${UUSPOOL}/${site}
  fi

  if [ ! -d ${UUSPOOL}/${site}/${DEFQUEUE} ] ; then
    mkdir ${UUSPOOL}/${site}/${DEFQUEUE}
  fi

  j=$( expr $site : '\(.\{1,7\}\)' )
  touch ${UUSPOOL}/${site}/${DEFQUEUE}/C.${j}${DEFQUEUE}0000

done

PROG=${UUROOT}/bin/uusched
if [ -x $PROG ] ; then
  ${UUROOT}/bin/uusched &
else
  /usr/lib/uucp/uusched &
fi



