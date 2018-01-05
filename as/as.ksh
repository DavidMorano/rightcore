#!/usr/bin/ksh
# AS

: ${CCS:=/usr/ccs}

POPTS=
RF_XARCH=false
for A in "${@}" ; do
  POPTS="${POPTS} ${A}"
  case "${A}" in
  -xarch*)
    RF_XARCH=true
    ;;
  esac
done
if ${RF_XARCH} ; then :
else
  POPTS="-xarch=v8plusa ${POPTS}"
fi

print -- ${CCS}/bin/as ${POPTS}

