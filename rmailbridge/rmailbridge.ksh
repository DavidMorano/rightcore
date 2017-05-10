#!/usr/bin/ksh
# RMAILBRIDGE


: ${LOCAL:=/usr/add-on/local}
: ${PCS:=/usr/add-on/pcs}
: ${POSTFIX:=/usr/postfix}
export LOCAL PCS POSTFIX


if [[ ! -d "${FPATH}" ]] ; then
  FPATH=${PCS}/fbin
fi
if [[ ! -d "${FPATH}" ]] ; then
  FPATH=${LOCAL}/fbin
fi
export FPATH

pathadd PATH ${PCS}/bin
pathadd LD_LIBRARY_PATH ${PCS}/lib

pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib


OPTS=
if [[ -n "${UUMACHINE}" ]] && [[ -n "${UU_USER)" ]] ; then
  OPTS="${OPTS} -f ${UU_MACHINE}!${UU_USER}"
fi

${POSTFIX}/bin/sendmail -B 8BITMIME ${OPTS} -oi "${@}"



