MAKESAFE

This command will checks the dependencies of a given object file to see if any
of them or any of their dependencies are newer than the given object file.
Normal language dependencies checked for object files are: C, C++, FORTRAN,
YACC, and header files. If any dependent files , eithr directly or indirectly,
dependent are newer than the given object file, the object file gets deleted.

Synopsis:
$ makesafe [-I <incdir(s)>] [-t <target>] [<objfile(s)>] [-v[=<n>]] [-z] [-V]

Arguments;
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

