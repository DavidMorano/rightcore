# CUTCOLS

function cutcols {
  if [[ $# -gt 0 ]] ; then
    for A in "${@}" ; do
      if [[ -r "${A}" ]] ; then
        expand ${A}
      fi
    done
  else
    expand 
  fi | cut -c 1-${COLUMNS:-80}
}


