EDITSTR

This program edits the specified files making string substitutions as specified.

Synopsis:
$ editstr -s <strfile(s)> [<file(s)>] [<dir(s)>] [-af <argfile>] 
		[-r <reqstr(s)>}
		[-nice <val>] [-s <suffix(es)>] [-v[=n]] [-n] [-V]

Arguments:
-s <strfile>	file of strings to use for processing
<file(s)>	file(s) to process
<dir(s)>	directories of file(s) to process
-af <argfile>	file specifying list of files or directories
-r <reqstr(s)>	requisite string to get a match
-nice <val>	nice value
-s <suffix(es)>	only process files with a suffix from this list
-v[=n]		verbosity level
-n		perform analysis but do not do any processing
-V		print program version to standard-error and then exit

Example:
$ editstr -s strfile *.c 

