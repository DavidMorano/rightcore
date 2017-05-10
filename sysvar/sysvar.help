SYSVAR

This program sets or reads the "system variables."  These are generally
variables that contain system initialization or confiuration data.

Synopsis for administrative use (setting variables):
$ sysvar [-s] [-f <file(s)] [-o audit] [-V]

Synopsis for user use (retrieving variables):
$ sysvar [-l[=<b>]] [<name(s)>|-a] [-o audit] [-V]

Arguments:
-s		sets the system variables into the database
-f <file(s)>	optional file(s) containing variables to set into the database
<name(s)>	variable name(s) to retrieve and print to standard-output
-a		print all variables to standard-output
-l[=<b>]	list all variables to standard-output (key=value pairs)
-o <opt(s)>	options:
			audit	- perform an audit on the underlying database
-V		print command version to standard-error and then exit

With no arguments, the system variables are read out of the database.  Setting
of the system variables should usually (only) be done at machine boot-up time.
System variables are also retained across system reboots, so a failure to set
variables at boot-time is not absolutely required (if the variables have been
set previously).

Also, when the system variables are read out without them having been set
previously, a default set of system variables are installed.

Use of the set-argument option creates a database that is read-only.  This is
intended for use at system boot-up time so that the database is not changed
subsequently by some other user.  Setting of the system variables may require
administrator priviledges if the variables have already been set by default.

