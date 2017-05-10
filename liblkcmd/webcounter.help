WEBCOUNTER

This program accesses (manages) a web counter.  A web counter database must be
specified somehow (either as an invocation argument or as an environment
variable).  Multiple counters are (or can be) stored in a single database.
Counters are identified by name (starting with an ASCII character).

Synopsis:
$ webcounter [-b <basedir>] [-qs <rs>] [-db <database>] [<name(s)>[=<v>]]
	[-af <afile>] [-l] [-h] [-b <basedb>] [-o <opt(s)>]
	[-lf <file>] [-C <conf>] 
	[-i[=<b>]] [-p[=<b>]] [-a[=<b>]] [-D] [-Q] [-V]

Arguments:
-b <basedir>	base-directory
-qs <qs>	query-string
-db <database>	the database file
<name(s)>[=<v>]	name of counter to print or increment w/ optional new value
-af <afile>	argument-list file of <name(s)>
-l		list all counters in a database (one per line)
-h		add a header when using the list option
-b <basedb>	base DB directory
-o <opt(s)>	options: log=<b>, basedb=<dir>, logsize=<n>
-i[=<b>]	increment the named counter (default)
-p[=<b>]	print the previous value of the named counter
-a[=<b>]	add named counter to DB if not already present (default)
-lf <file>	log-file
-C <conf>	configuration file
-Q		do not report some non-fatal errors
-V		print command version to standard-error and then exit

