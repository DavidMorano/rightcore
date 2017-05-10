for i
do if [ -r $i ]
   then CALFILE="${CALFILE} $i"
   else DATE="$DATE $i"
   fi
done
tmp=/tmp/cal$$
trap "rm $tmp; trap '' 0; exit" 0 1 2 13 15
/gaz/lib/calprog $DATE >$tmp
cat ${CALFILE-/gaz/lib/calendar} | fgrep -f $tmp 
