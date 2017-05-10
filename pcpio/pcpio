#!/usr/bin/ksh
# PCPIO (POSIX CPIO)


: ${SYSNAME:=$( uname -s )}
: ${RELEASE:=$( uname -r )}

case ${SYSNAME}:${RELEASE} in

SunOS:5* )
  P_CPIO=/usr/bin/cpio
  OPTS="-H odc"
  ;;

SunOS:4* )
  P_CPIO=/usr/bin/cpio
  OPTS="-c"
  ;;

OSF*:* )
  P_CPIO=/usr/bin/cpio
  OPTS="-c"
  ;;

* )
  P_CPIO=/usr/bin/cpio
  OPTS=
  ;;

esac

exec ${P_CPIO} ${OPTS} "${@}"



