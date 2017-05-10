# rem_add: adds new items to reminder file
# and removes duplicates
# J. A. Kutsch  ho 43233  x-3059
# July 1980
#
# rem_add is riun by the reminder daemon to merge new items into
# the master reminder file
#
REMDIR=/usr/ncmp/pcs/lib/remdata

umask 0000
cd $REMDIR
set rem.*
if test $# -gt 1 -o "$1" != 'rem.*'
then
cat remfile $* > remtemp
rm -f $*
sort < remtemp | uniq > remfile
fi
