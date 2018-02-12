MKTAGPRINT

This program reads a section of a file specified by an index tag. One or more
index tags are read from standard input.  Index tahs are normally generated
as a result of queries, usually from the output of the MKQURY program.

Synopsis:
$ mktagprint [<tagfile(s)> ...] [-af <argfile>] 
	[-b <basedir>] [-f <outfmt>] [-V]

Arguments:
<tagfile(s)>	file containing tags
-af <argfile>	file containing filenames each containing tags
-b <basedir>	base directory to use for the index tags
-f <outfmt>	output format:
			raw
			fill
-V		print program version to standard-error and then exit

