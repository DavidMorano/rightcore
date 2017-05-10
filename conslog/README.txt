CONSLOG

This is a KSH shell built-in command.  It assists in writing to the
console-logging mechanism.

Synopsis:
$ conslog [<fac>]:<pri>]] [-f <fac>] [-p [<fac>:]<pri>]
	[-<pri>] [-o <option(s)>] [-if <infile>] [-V]

Arguments:
<fac>[<pri>		facility and optional priority
-f <fac>		facility (default 'user')
-p [<fac>:]<pri>	priority <pri> w/ option facility <fac>
-<pri>			priority in decimal
-o <option(s)>		one of: check
-if <infile>		source data file (default is STDIN)
-V			print command version to standard-error and then exit

