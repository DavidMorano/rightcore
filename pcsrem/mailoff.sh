# mailoff: removes user from mail notification list
# J. A. Kutsch  ho 43233  x-3059
# January 1981
#
#
REMDIR=/usr/ncmp/pcs/lib/remdata
cd $REMDIR
if grep "^$LOGNAME$" mc_users > /dev/null
then ed mc_users > /dev/null <<!
g/^$LOGNAME$/d
w
q
!
echo Mail notification is now inactive
else
echo Mail notification was not active
fi
