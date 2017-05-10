: set -xv		# debugging statement
#
#   NAME
#	rem_mon - reminder service daemon initialization
#
#   SYNOPSIS
#	rem_mon [ rc | cron ]
#
#   DESCRIPTION
#	Rem_mon initializes things for the reminder service daemon.
#	It should be called from /etc/rc with the "rc" option.
#	It should be called from /usr/lib/crontab with the "cron" option.
#
#   AUTHORS
#	J. A. Kutsch  ho 43233  x-3059
#	November 1980
#
#	Tony Hansen LZ 3B-315, x3044
#	March 1983

# Save the argument for later
RC="$1"

# Go to the right place
REMDIR=/usr/ncmp/pcs/lib/remdata
cd $REMDIR

# first, remove TMP* files from aborted pgms
rm -f TMP*

# If reminder file was not accessed today, clear it.
# Or if we're called from /etc/rc or /usr/lib/crontab.
set  -- `ls -l remfile`
fdate=$6$7
set -- `date`
cdate=$2$3
if [ "$RC" = rc -o "$RC" = cron -o $fdate != $cdate ]
then
    > remfile	#clear it
fi

# If we're being called from /etc/rc, just start the demon!
# if rem_pgm is not running, start it
if [ "$RC" = rc -o `ps -e | grep -c rem_pgm` = 0 ]
then
    ./rem_pgm
fi
