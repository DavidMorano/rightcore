#!/usr/bin/ksh
# MAN

SUNMAN=/usr/bin/man

#
# This is a special 'man' such that the Sun Solaris versions of 'nroff' and
# 'eqn' get called. These are required for some of Sun's new manual pages.
#

: ${DWBHOME:=/usr/add-on/dwb}
export DWBHOME

#PATH=${DWBHOME}/bin:${PATH}
PATH=/usr/bin:${PATH}
export PATH

${SUNMAN} "${@}"
rm -f /tmp/mp*


