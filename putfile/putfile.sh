#!/bin/sh
# PUTFILE


if [ ! -r "${1}" ] ; then
  echo "${0}: no file name was specified" >&2
  exit 1
fi

exec /bin/cat > $1


