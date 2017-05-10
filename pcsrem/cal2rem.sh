# cal2rem: move from calendar to reminder service
# J. A. Kutsch  ho 43233  x-3059
# July 1980
# search calendar for items with today's date and a time stamp.
# pass these items to 'rem' (reminder service) in appropriate format.
# Usage: cal2rem calendar
#
if test $1
then
if test -f $1
then
pcscal +r $1 | rem -
else echo "usage: cal2rem calendar"
fi
else echo "usage: cal2rem calendar"
fi

