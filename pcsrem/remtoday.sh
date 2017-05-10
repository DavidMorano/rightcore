# Print out all reminders for me for today
# Charlie Shub and Neal McBurnett, 7/82
# Modified by Carole Strada, 8/23

TMPFILE=/tmp/remtoday$$
DATE=`date '+%a %h %d'`
trap "rm ${TMPFILE} > /dev/null 2>/dev/null;exit" 1 2 3 15

REMTMP=`echo $PCSLIB/remdata/rem.a*`
if test "$REMTMP" = "$PCSLIB/remdata/rem.a*"
then
	REMTMP=""
fi

cat $REMPATH $REMTMP | grep $LOGNAME | sed "s/.\{15\}\(..\)\(..\) */\1:\2 /" |  sed "s/AM *//" | sed "s/NOON *//" | sed "s/PM *//" | sort | uniq > ${TMPFILE} 2>/dev/null
if test  -s ${TMPFILE}
then
ed - ${TMPFILE} <<!
g/^/s//		/
w
q
!
cat ${TMPFILE}


else	echo "no reminders for $DATE"
fi

rm ${TMPFILE}



