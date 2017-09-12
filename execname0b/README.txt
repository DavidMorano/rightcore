EXECNAME

This program is used a lot in these SGS packages to change the name under which
a program executes. It is almost identical with the 'execv' program distributed
with the AT&T EXPTOOLS package but is a little more flexible than that program.

Synopsis:
# execname -V <progpath> [<progname>|+|-] [<arg(s)>]

Arguments:
<progpath>	program path (if not absolute, a search is done)
<progname>	name under which program will run (this will be 'argv[0]')
+		make name of program base-name of <progpath>
-		make name of program base-name of <progpath= w/ '-' prepended
<arg(s)>	optional arguments to program
-V		print program version to standard-error and then exit


