#!/bin/ksh

PROGDIR=${0%/*}
A=${0##*/}
PROGNAME=${A%.*}

echo PROGDIR=${PROGDIR}
echo PROGNAME=${PROGNAME}

echoargs "${@}"

echo LOCAL=${LOCAL}



