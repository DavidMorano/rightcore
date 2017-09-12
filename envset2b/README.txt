ENVSET

This program runs another program with am arranged environment. Usually the idea
is to setup a default environment for execution contexts which do not have an
environment expressive enough to access all facilities of the system. This is
often run as the LOGIN shell in diguise (usually as LSH in this mode) in order
to setup the desired environment for all new sessions on the system.  It is 
very useful for commands within CRONTABs also.

Symopsis:
$ envset [-df <def>] [-xf <xe>] [<other(s)>] <prog> {<arg0>|+|-} [<argn> ...]]

Arguments:
-df <def>	definition file
-xf <xe>	eXportable Environment file
<other(s)>	any of:
			-nice <n>
<prog>		program executable to execute
<arg0>		first argument to the program
+		first argument will be base-name of <prog>
-		first argument will be base-name of <prog> w/ '-' prepended
<argn>		optioanl remaining argument(s)

Some standard environment variables are created or retained under normal
circumstances.

