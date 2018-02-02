#!/usr/bin/ksh
# SUM

integer SUM=0 C
while read A B ; do
  (( SUM = SUM + ${B} ))
  (( C = ${A} + 1 ))
  print ${C}0101 ${SUM}
done < oo.txt


