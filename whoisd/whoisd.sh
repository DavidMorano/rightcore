#!/usr/bin/ksh
# WHOISD


LOGFILE=/tmp/whoisd.log

: ${PCS=:/usr/add-on/pcs}

DATE=$( date '+%y%m%d_%T' )

if [ ! -w $LOGFILE ] ; then
  echo "${DATE} whoisd started" > $LOGFILE
  chmod 666 $LOGFILE
fi

Q=$( line )
PN=$( ${PCS}/bin/pcsconf peername )

echo "${DATE} from=${PN}" >> $LOGFILE
echo "q=>${Q}<" >> $LOGFILE

echo "you have reached the RightCore Power Macintosh server"


