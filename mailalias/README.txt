MAILALIAS

This progam takes mail-alias names as input (or invocation arguments) and print
their expansions to standard-output. By default, the system default mail-alias
profile configuration (named 'default') is used to determine how aliases are
looked up and expanded.

Synopsis:
$ mailalias [-p <profile>] [<aliasname(s)> ...] [-af <afile>] 
	[-dump <dfile>] [-V]

Arguments:
-p <profile>	mail-alias profile (default is 'default')
<aliasname(s)>	names to translate
-af <afile>	argument list file
-dump <dfile>	dump the database in ASCII to file
-V		print command version to standard-error and then exit

