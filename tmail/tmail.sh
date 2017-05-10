#!/bin/ksh
# TMAIL (test mail)


LOG=/home/dam/rje/tmail.log

DATE=`date '+%y/%m/%d %T' `

exec >> $LOG

echo
echo "*** tmail new message ${DATE}"
/home/dam/bin/echoarg "${@}"
echo "*** body"
exec cat 



