#!/usr/extra/bin/ksh
# SUBTESTISSUE

ISSUE=/etc/issue

builtin -f lkcmd shcat

integer i j k

for (( i = 0 ; i < 100 ; i += 1)) ; do
  for (( j = 0 ; j < 100 ; j += 1)) ; do
    shcat ${ISSUE}
  done &
done
wait


