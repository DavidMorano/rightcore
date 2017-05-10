#!/usr/bin/ksh
# CLUSTERNODES



DN=/dev/null
if whence pingstat > ${DN} && whence clustername > ${DN} ; then
  : ${CLUSTERNAME:=$( clustername )}
  if [[ -n "${CLUSTERNAME}" ]] ; then
    pingstat -p ${CLUSTERNAME} | grep '^U' | cut -c 3-25 | while read N J ; do
      NC=$( clustername ${N} )
      if [[ ${NC} == ${CLUSTERNAME} ]] ; then
        print ${N}
      fi
    done
  fi
fi



