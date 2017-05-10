#!/usr/extra/bin/ksh
# CHFILE
#
#
# Synopsis:
#
#	$ chfile [-g <group>] [-m <acl>] [files(s)]
#
# Notes:
#
#	This program contains KSH specific language elements and it
#	MUST be executed by KSH!
#
#
# copyright:
# /* Copyright © 1998 David A­D­ Morano.  All rights reserved. */
#
#
# revision history:
#
# = 2017-01-11, David Morano
#	This program was written for Rightcore Network Services.
#


RF_DEBUG=false

FILES=
FMODE=
FGRP=


S=files
OS=""
for A in "${@}" ; do
  case ${A} in
  '-D' )
    RF_DEBUG=true
    ;;
  '-g' )
    OS=${S}
    S=fgrp
    ;;
  '-m' )
    OS=${S}
    S=fmode
    ;;
  '-'* )
    print -u2 "${PN}: unknown option \"${A}\""
    exit 1
    ;;
  * )
    case ${S} in
    files )
      FILES="${FILES} ${A}"
      ;;
    fgrp )
      FGRP=${A}
      S=${OS}
      ;;
    fmode )
      FMODE=${A}
      S=${OS}
      ;;
    esac
    ;;
  esac
done

for F in ${FILES} ; do
  if [[ -n "${F}" ]] ; then
    if [[ -f ${F} ]] ; then
      if [[ -n "${FGRP}" ]] ; then
        chgrp ${FGRP} ${F}
      fi
      if [[ -n "${FMODE}" ]] ; then
	chmod ${FMODE} ${F}
      fi
    fi
  fi
done


