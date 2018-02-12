PINGSTAT

This program is used to maintaince PING status with network hosts.

Synopsis:
$ pingstat [-u] [<host(s)>] [-p <pingtab(s)>] [-t <to>] [-q]
	[-dgram] [-i[=<minlen>]] [-m <minpoll>] [-v[=<v>]] [-V]

Arguments:
-u		update mode
<host(s)>	network hosts to update
-pd <dir>	specify directory for ping-tab files
-p <pingtab>	file tables contain host specifications
-t <to>		ping timeout
-q		do not print any output (only provide exit code)
-dgram		specify data-gram mode (for daemon input mode)
-i[=<minlen>]	input mode w/ optional minimum input length
-m <minpoll>	specifiy the minimum time between updates
-db <db>	use this database
-v[=<lev>]	verbosity level <lev>
-V		print program version to standard-error and then exit

