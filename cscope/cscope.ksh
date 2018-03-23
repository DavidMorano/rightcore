#!/usr/extra/bin/ksh
# CSCOPE


: ${SUNWSPRO:=/opt/SUNWspro}
export SUNWSPRO

if [[ -n "${CSCOPE_EDITOR}" ]] ; then
  EDITOR=${CSCOPE_EDITOR}
elif [[ -n "${VIEWER}" ]] ; then
  EDITOR=${VIEWER}
else
  EDITOR=vi
fi
export EDITOR

if [[ -d "${SUNWSPRO}" ]] && [[ -d ${SUNWSPRO}/bin ]] ; then
  ${SUNWSPRO}/bin/cscope "${@}"
else
  cscope "${@}"
fi


