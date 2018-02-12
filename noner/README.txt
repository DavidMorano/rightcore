NONER

This program takes a list of files on its standard-input and prints out those
files that contain weirdo characters in them.

Synopsis:
$ noner [<file(s)>] [-af <argfile>] 
	[-s <suffix(es)>] [-f] [-v[=n]] [-V]

Arguments:
<file(s)>	directories (or files) to include in calculation
-af <argfile>	file specifying list of files or directories
-s <suffix(es)>	only process files with a suffix from this list
-v[=n]		verbosity level
-f		follow symbolic links
-V		print program version to standard-error and then exit 

