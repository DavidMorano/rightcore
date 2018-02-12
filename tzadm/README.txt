TZADM

This program is used to administer the in-memory version of the TIMEZONE
database. It is primarily meant to be run by a software-distribution
administrator at system boot-up time in order to load the database into machine
memory for the first time (since a boot-up).

Synopsis:
$ tzad [-db <name>] [-r] [<dbfile>] [-g <zname>] [-l] [-o <opts>] [-V]

Arguments:
-db <name>	name given to the in-memory database
-r		indicate that a reload is desired (even if already loaded)
<dbfile>	filename to load into memory (else a default is used)
-g <zame>	query the in-memory database for <zname>
-l		list all timezones (w/ information) to standard-output
-o <opts>	options:
			audit	- performs an audit on in-memory DB
-V		print program version on standard-error and then exit

