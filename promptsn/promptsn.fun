# PROMPTSN

function promptsn {
  typeset N
  : ${NODE:=$( nodename )}
  case "${NODE}" in
  rc*)
    N=${NODE:2:10}
    ;;
  *)
    N=${NODE}
    ;;
  esac
  if [[ -n "${WINDOW}" ]] ; then
    N=${N}:${WINDOW}
  fi
  PS1="${N}> "
}


