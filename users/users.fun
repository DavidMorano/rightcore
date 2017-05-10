# USERS


function users_unique {
  typeset EX LIST A TU
  EX=0
  LIST="${1}"
  A=${2}
  #print -u2 LIST=\"${LIST}\" A=${A}
  for TU in ${LIST} ; do
    #print -u2 TU=${TU}
    if [[ "${A}" == ${TU} ]] ; then
      EX=1
      break
    fi
  done
  return ${EX}
}

function users {
  typeset EX LIST U J
  integer c=0
  LIST=
  if whence wn > /dev/null ; then
    wn -nh -l | while read U J ; do
      if users_unique "${LIST}" ${U} ; then
        LIST="${U} ${LIST}"
        (( c += 1 ))
      fi
    done
  fi
  print -- ${LIST}
  EX=1
  if (( ${c} > 0 )) ; then
    EX=0
  fi
  return ${EX}
}

if whence builtin > /dev/null ; then
  function USERS.get {
    if whence users > /dev/null ; then
      .sh.value=$( users )
    fi
  }
fi


