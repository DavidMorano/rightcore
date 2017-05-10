%{SS}

This command tests if a specified process has some specified attributes.

Synopsis:
$ %{S} [-pid <pidfile(s)>] [-af <afile>] [<pid(s)>] [-q <query(s)>] 
		[-<age>] [-V]

Arguments:
<pid(s)>	PIDs to check
-af <afile>	argument-list file
-pid <file(s)>	PID file(s) to check
-q <query(s)>	query(s): running (default), notrunning
-<age>		age of <pidfile(s)>
-V		print command version to standard-error and then exit

Returns:
0		all criteria met
>0		some criteria not met

Example:
$ isproc -q notrunning -pid ${EXTRA}/var/run/xinetd.pid -5m

