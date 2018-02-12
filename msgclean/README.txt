MSGCLEAN

This program will fix up (clean up) message files that have permissive
address specification header keys.

Synopsis:
$ msgclean [<file(s)>] [<dir(s)>] [-af <afile>] [-t <ageint>] 
	[-nice <val>] [-s <suffix(es)>] [-v[=n]] [-V]

Arguments:
<file(s)>	directories (or files) to include in calculation
-af <afile>	file specifying list of files or directories
-t <ageint>	age-interval in seconds
-nice <val>	nice value
-s <suffix(es)>	only process files with a suffix from this list
-v[=n]		verbosity level
-f		follow symbolic links
-n		perform analysis but do not do any processing
-V		print program version to standard-error and then exit 

Example:
$ msgclean *.msg -t 30

