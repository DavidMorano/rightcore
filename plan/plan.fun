# PLAN (function)

function plan {
  if [[ -z "${1}" ]] ; then
    : ${USERNAME:=$( username )}
    rfinger ${USERNAME}
  else
    rfinger "${@}"
  fi
}


