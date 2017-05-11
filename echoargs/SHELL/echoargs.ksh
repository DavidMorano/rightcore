#!/usr/bin/ksh
# ECHOARGS


integer c

(( c = 0 ))
echo "${c}> ${0}"

for A in "${@}" ; do
  (( c += 1 ))
  echo "${c}> ${A}"
done

