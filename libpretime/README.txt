LIBTIME

This library is provided in order to intercept the some of the UNIX® System
time related subroutines.  This alows for the manipulation of the
time-of-day, mostl for program testing purposes.

The UNIX® calls that are currently intercepted are:

+ time
+ gettimeofday
+ ftime
+ clock_gettime

Calls that may need to be intercepted in the future are:

+ clock_getime (CLOCK_REALTIME)

= Activate with:

export LD_PRELOAD=/usr/lib/secure/libpretime.so
export LIBPRETIME_BASETIME=<toucht>

