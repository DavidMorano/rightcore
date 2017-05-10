#!/usr/bin/ksh
# WN


: ${LOCAL:=/usr/add-on/local}
: ${AST:=/usr/add-on/ast}
export LOCAL AST


#print -u2 $0
#print -u2 ${.sh.version}


TFI=/tmp/wni${$}
TFO=/tmp/wno${$}

function cleanup {
  rm -f $TFI $TFO
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

N=/dev/null
cat <<EOF > $TFI
if whence builtin > $N ; then
  if [[ -r "${ENV}" ]] ; then
    . ${ENV}
  fi
  if builtin | grep "^wn" > $N ; then
    EX=$?
#  print -u2 "got it >${@}< ex=${EX}"
    wn -of ${TFO} ${*} 
    cat $TFO
  fi
fi
EOF

#cat $TFI > oo
${AST}/bin/ksh ${TFI}

cleanup


