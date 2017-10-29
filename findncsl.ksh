#!/usr/extra/bin/ksh
# FINDNCSL


function proc_wc {
  typeset C N BN
  integer sum=0
  while read C N ; do
    BN=${N##*/}
    if [[ "${BN}" != "total" ]] ; then
      (( sum += ${C} ))
    fi
  done
  print -- ICSL ${sum}
}

function proc_ncsl {
  typeset C N BN
  integer sum=0
  while read C N ; do
    BN=${N##*/}
    if [[ "${BN}" != "total" ]] ; then
      (( sum += ${C} ))
    fi
  done
  print -- NCSL ${sum}
}

function process {
  filefind -t f -s c,h,cc -o uniq ${*} > ${TF}
  xargs < ${TF} wc -l | proc_wc
  xargs < ${TF} ncsl -s | proc_ncsl
  rm -f ${TF}
}

TF=/tmp/findncsl${$}

function cleanup {
  rm -f ${TF}
}

trap 'cleanup;exit 1' 1 2 3 15 16 17

SRC=${*}
if [[ -n "${SRC}" ]] ; then
  process ${SRC}
else
  process *
fi

rm -f ${TF}


