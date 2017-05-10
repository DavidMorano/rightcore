#!/usr/bin/ksh
# LINT


SUNWSPRO=/opt/SUNWspro

export PATH=${PATH}:${SUNWSPRO}/bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${SUNWSPRO}/lib

${SUNWSPRO}/bin/lint "${@}"


