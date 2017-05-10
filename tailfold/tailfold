#!/usr/bin/ksh
# TAILFOLD

PN=tailfold

: ${LOCAL:=/usr/add-on/local}
export LOCAL

PATH=${PATH}:${LOCAL}/bin
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${LOCAL}/lib
export PATH LD_LIBRARY_PATH

SUF=${$}

TD=$( mktmpuser tailfold )

TF=${TD}/tf${$}
TSXF=${TD}/tsxf${$}

function cleanup {
  rm -f ${TF}
  rmfile -d=60 ${TSXF}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

D=d${SUF}
E=e${SUF}

# exclusions
print gethostby > ${TSXF}

TAILE_OPTS="sxf=${TSXF},fold,clean"
export TAILE_OPTS

cat > ${TF}
taile -af - "${@}" < ${TF} &

sleep 1
cleanup



