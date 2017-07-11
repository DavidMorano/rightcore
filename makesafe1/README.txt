MAKESAFE

This command will check all of the C language source files for all of the
include files that they use. It will then check if any include file is newer
than the corresponding C language object file if one exists. If any include file
is newer than the object file, the object is removed.

Synopsis:

$ makesafe [-I <incdir(s)>] [-t <target>] [<objfile(s)>] [-v[=<n>]] [-z] [-V]

where:

<objfile(s)>	file to check for safety
-af <argfile>	argument list file
-I <incdir(s)>	directories to search for "include" files
-t <target>	file to "touch" when completed
-v[=<n>]	verbosity level
-o <opts>	where options are:
			cpp=<prog>	CPP program
			cache[=<n>]	set cache ON or OFF (default is ON)
			par=<n>		set parallelism
-z		do not complain about no files specified
-npar		number of paralleln processors employed
-V		print command version to standard error and then exit

