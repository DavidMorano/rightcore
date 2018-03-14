#!/usr/extra/bin/ksh
# MKMANPDF

DEV=Latin1

for F in *.man ; do
  BF=${F##*/}
  M=${BF%.man}
  troff -T${DEV} -man ${M}.man | dpost -T${DEV} -x0.25 > ${M}.ps
  ps2pdf ${M}.ps ${M}.pdf
done


