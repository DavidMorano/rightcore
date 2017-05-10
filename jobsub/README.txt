JOBSUB

This is a batch job execution management system.  It is very hacky!
It most closely resembles 'batch(1)' from standard UNIX.  The server
for standard 'batch(1)' is 'cron(1m)'.  The server for this faciity is
itself.

Synopsis:

$ jobsub [-q queuename] < jobfile [-v] [-m[=mailaddress]]

where:

-q queuename		queue to place job in (default 'b')
-m=mailaddress		send mail on completion to address
jobfile			file w/ SHELL commands

