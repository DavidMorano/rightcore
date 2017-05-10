#!/usr/extra/bin/ksh
# FPATHCHECK

BLIB=liblkcmd.so

: ${LOCAL:=/usr/add-on/local}
export LOCAL

DN=/dev/null

if whence builtin > ${DN} ; then
  BLIBDIR=${LOCAL}/lib
  builtin -f ${BLIBDIR}/${BLIB} isfile
fi

OIFS=${IFS}
IFS=:
for D in ${FPATH} ; do
  if [[ -n "${D}" ]] && [[ -d "${D}" ]] ; then 
     ls ${D} | while read E ; do
       F=${D}/${E}
       if [[ -x ${F} ]] ; then
	 print -- executable ${F}
       elif isfile -obj ${F} ; then
	 print -- object ${F}
       elif isfile -bin ${F} ; then
	 print -- binary ${F}
       fi
     done
  fi
done
IFS=${OIFS}


