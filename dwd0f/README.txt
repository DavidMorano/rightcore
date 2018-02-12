DWD

This program will watch a single directory for incoming files and look up the
file in a "service" table to find the appropriate server program to spawn off to
handle the request.

NOTICE:	Regular expressions are not available when compiled under
	BSD UNIX.  BSD UNIX does not have the proper regular expression
	subroutines.

This is an oldie. It's roots go way back. Although it is old, it is still in
operation for some purposes. You can reference the 'CHANGES' file for change
information. Unfortunately, the new 'DW' object, that was supposed to make this
old daemon obsolete some day, never quite came along as expected! It's an old
story! What remains true is that old programs tend to stay in operation much,
much longer than ever expected, no matter how much we would like otherwise!

Synopsis:
$ dwd [-c <server>] [<directory> [<srvtab>]] [-v] [-q] 
		[-P <pidfile>] [-C <configfile>] [-V]

A note on maintainability of this program:

This program is a mess! It is exceedingly difficult to either figure out what is
going on or to ever attempt to modify this program. Yes, there are some objects
but there are not enough and one or more of them are not clean enough. Touch
this beast at your own risk.

