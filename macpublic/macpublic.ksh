#!/usr/bin/ksh
# MACPUBLIC


# this little program (must be run as root) checks to make sure that
# all users -- located in the '/Users' directory -- have have:
#	"Public"
#	"Public/Drop Box"
#	"rje"
#
# all both existing and with the proper permissions.
#
#
#


USERS=/Users


: ${SYSNAME:=$( uname -s )}
export SYSNAME


: ${EXTRA:=/usr/extra}

if [[ ! -d "${EXTRA}" ]] ; then
  exit 1
fi


PATH=${PATH}:${EXTRA}/bin:${EXTRA}/sbin
export PATH

case "${SYSNAME}" in
SunOS)
  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${EXTRA}/lib
  export DYLD_LIBRARY_PATH
  ;;
Darwin)
  DYLD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${EXTRA}/lib
  export DYLD_LIBRARY_PATH
  ;;
esac


if [[ ! -d "${USERS}" ]] ; then
  exit 0 ;
fi

cd ${USERS}
if [[ "${PWD}" != "${USERS}" ]] ; then
  exit 1
fi

for U in * ; do
  if [[ -d "${U}" ]] && isuser $U ; then
    D=${U}/rje
    if [[ ! -d "${D}" ]] ; then
      mkdir "${D}"
    fi
    if [[ -d "${D}" ]] ; then
      chmod ugo+rwx "${D}" ; chown ${U} "${D}"
    fi
    D=${U}/Public
    if [[ ! -d "${D}" ]] ; then
      mkdir "${D}"
    fi
    if [[ -d "${D}" ]] ; then
      chmod ugo+rx "${D}" ; chown ${U} "${D}"
    fi
    D="${U}/Public/Drop Box"
    if [[ ! -d "${D}" ]] ; then
      mkdir "${D}"
    fi
    if [[ -d "${D}" ]] ; then
      chmod ug+rwx,o+wx "${D}" ; chown ${U} "${D}"
    fi
  fi
done



