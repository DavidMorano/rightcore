#!/bin/sh
# EXECNAME


#echo "${0}: args> ${*}" >&2


EXECNAME_LOCALETCBIN=${LOCAL:-/usr/add-on/local}/etc/bin
EXECNAME_LOCALBIN=${LOCAL:-/usr/add-on/local}/bin
EXECNAME_PCSETCBIN=${PCS:-/usr/add-on/pcs}/etc/bin
EXECNAME_PCSBIN=${PCS:-/usr/add-on/pcs}/bin
EXECNAME_HOMEBIN=${HOME}/bin


EXECNAME_BINS="${EXECNAME_LOCALETCBIN} ${EXECNAME_LOCALBIN}"
EXECNAME_BINS="${EXECNAME_BINS} ${EXECNAME_PCSETCBIN} ${EXECNAME_PCSBIN}"
EXECNAME_BINS="${EXECNAME_BINS} ${EXECNAME_HOMEBIN}"


OS=`uname -s`
ISA=`uname -p`


for EXECNAME_BIN in $EXECNAME_BINS ; do


if [ -d $EXECNAME_BIN ] ; then


case ${OS}:${ISA} in

SunOS:sparc )

if [ -x /usr/sbin ] ; then

  if [ -x ${EXECNAME_BIN}/execname.elf ] ; then 
	exec ${EXECNAME_BIN}/execname.elf "${@}" ; fi

  if [ -x ${EXECNAME_BIN}/execname.x ] ; then 
	exec ${EXECNAME_BIN}/execname.x "${@}" ; fi

  if [ -x ${EXECNAME_BIN}/execname.aout ] ; then 
	exec ${EXECNAME_BIN}/execname.aout "${@}" ; fi

else

  if [ -x ${EXECNAME_BIN}/execname.aout ] ; then 
	exec ${EXECNAME_BIN}/execname.aout "${@}" ; fi

  if [ -x ${EXECNAME_BIN}/execname.x ] ; then 
	exec ${EXECNAME_BIN}/execname.x "${@}" ; fi

fi
  ;;

OSF*:alpha )
  if [ -x ${EXECNAME_BIN}/execname.osf ] ; then 
	exec ${EXECNAME_BIN}/execname.osf "${@}"
  fi
  ;;

esac

fi


done

# we couldn't find any EXECNAME programs around !!


EXECNAME_PATH=$1
shift
EXECNAME_NAME=$1
shift

exec $EXECNAME_PATH "${@}"


echo "${0}: could not find underlying EXECNAME program" >&2
exit 1



