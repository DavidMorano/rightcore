PCSULM (PCS User Location Manager)

This is the User Location Manager program for the PCS system.  It
tracks where users are located for possible notification of them by the
PCS system on certain events.

Synopsis:
$ pcsulm -d[=<runtime>]

This runs the program as a daemon.  Only one daemon program can run on
a host or network cluster at the same (based on the scope of the PID
lock or the file lock).  Note that having more than one isn't a problem
either since all database operations are synchronized among all
possible daemons.

It can also be run as a poll in the form:
$ pcsulm

This form also creates a user login-record for the calling user (if she
is logged in).  This form (like the daemon form above) will do a scan
poll of all records and carry out any actions that have come due.  This
form can also be run from a CRONTAB.  If a daemon is already
running on the same host or network cluster, then only a login-record
is generated and processed and then the program exits.

The program is often run by programs that log the fact that their users
have logged in.  In this mode it is run as:
$ pcsulm -i 

with login-update record(s) on its standard input.

