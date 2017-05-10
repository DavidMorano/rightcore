#!/usr/extra/bin/ksh
# DOMAINS
#
#
# Synopsis:
#
#	domains 
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

DOMAINS=/etc/domains
if [[ -r "${DOMAINS}" ]] ; then
  while read LINE ; do
    case ${LINE} in
    '#'* | '' )
      ;;
    * )
      print -- ${LINE}
      ;;
    esac
  done < ${DOMAINS}
fi


