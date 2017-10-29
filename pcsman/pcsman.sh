#!/usr/extra/bin/ksh
# PCSMAN

# Program to print out the PCS manual pages
# initialize variables, change for local system

: ${PCS:=/usr/add-onpcs}

MANDIR=${PCS}/man

if [ $# -lt 1 ] ; then
	echo usage:  pcsman filename ...
	echo "
The available manual pages for PCS (Personal Communication Services) are:"
echo ""
	ls ${MANDIR}/man ${MANDIR}/cat|sed -e /\^$\/d -e /:/d -e s/.1\$//  \
		|sort -u | pr -t -l19 -5
echo ""
echo "
Manual pages from the above that include other PCS programs are:

bb (includes bbpost)		sendmail (includes pc,info,and maillists)
cal (includes calpost and qp)   profiler (includes standard startup)"
	exit
fi
for arg in $*
do
	if [ -r ${MANDIR}/cat/${arg}.1 ]
	then cat ${MANDIR}/cat/${arg}.1
	elif [ -r ${MANDIR}/man/${arg}.1 ]
	then nroff -man ${MANDIR}/man/${arg}.1
	else echo "There is not a PCS manual page for ${arg}"
	fi
done


