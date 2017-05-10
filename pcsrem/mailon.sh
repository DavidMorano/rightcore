# mailon: inserts user in mail notification list
# J. A. Kutsch  ho 43233  x-3059
# January 1981
#
#
REMDIR=/usr/ncmp/pcs/lib/remdata
cd $REMDIR
if grep "^$LOGNAME$" mc_users > /dev/null
then echo Mail notification was already active
else echo $LOGNAME >> mc_users
echo Mail notification is now active
fi
