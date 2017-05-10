%{SS}

This little program reads a list of filenames in on its standard input.  It
then writes the file names it finds except for the names that were specifled as
invocation arguments.  That is the basic filter function.

Synopsis:

$ %{S} [<name(s)>] [-af <argfile>] [-sa <sufacc(s)>] [-sr <sufrej(s)>]
	[-o <option(s)>] [-u] [-V]

where:

<name(s)>	filenames to ignore
-af <argfile>	file of filenames to ignore
-sa <sufacc(s)> suffix-accepts
-sr <sufrej(s)> suffix-rejects
-o <option(s)>	options to control what gets filtered out
			uniq
			name
			noprog
			nosock[et]
			nopipe
			nodev
-u		turn on unique filtering
-V		print program version to standard-error and then exit

