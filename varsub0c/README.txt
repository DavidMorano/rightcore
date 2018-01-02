VARSUB

This program performs variable substitution on one or more files. The result is
written to an output file or STDOUT.

Synopsis:
$ varsub [<file(s)> ...] [-af <afile>] [-k <keychars>] [-b[=<n>]] 
	[-o badnokey[=<v>],blanks[=<v>]] [-z] [-db <map>] [-s <var>=<value>] 
	[-env] [-V]

Argumuments:
<file(s)>		input file(s) to substitute variables in
-af <afile>		file w/ list of file(s) to substitute for
-k <keychars>		specifiy key characters
-b[=<n>]		perform blank substitution if no key available, 
			<n> is type
-o <options>		options include:
				badnokey[=<v>]	exit on a bad key
				blanks[=<v>]	don't allow blank substututions
-z			substitute nothing for non-existent keys
-db <map>		map DB file 
-s <var>=<value>	substitute the variable for its value (repeatable)
-env			use the environment for unknown keys
-V			print program version to standard-error and then exit

Notes:
The characters to introduce a substitution variable are:
	${}
or
	$()

