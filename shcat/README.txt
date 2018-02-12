SHCAT

This command is similar to the 'cat(1}' program except that it is (or can be)
builtin into the SHELL. Advantages of being built into the SHELL include the
fact that it can be "found" even when the filesystem that 'cat(1)' resides on
cannot be mounted!

Synopsis:
$ shcat [<file(s) ...>] [-af <argfile>] [-of <outfile>] 
    [-to <to_open>] [-tr <to_read>] [-V]

Arguments:
<file(s)>	file(s) to be concatenated together to the standard-output
-af <argfile>	file containing file names to read as input
-of <outfile>	write the output to this file instead of to standard-output
-to <to_open>	open time-out
-tr <to_read>	read time-out
-V		print command version to standard-error and then exit

