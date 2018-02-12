HOLIDAY

This is a KSH shell built-in command.  It queries the HOLIDAYS database
and prints out any results that are found.  Queries can be made in three
different ways (all results printed togehter).  Supported query types
are: month-day, current date, and key-name.  The key-name must match
the first word of the associated description for a holiday.

Synopsis:
$ holiday [<spec(s)> ...] [+<n>] [-af <afile>] [-<n>] [-n <name(s)>] 
	[-db <dbfile>] [-z[=<b>]] [-y <year>] [-V]

Arguments:
<spec(s)>	query specification: 
			<mon><day>	specific day of given month
					ex: 01/01 jan/01 jan1 jan-1 1-1
			+[<days>]	today plus optional additional days
+[<n>]		query today and optional the specified additional days
-af <afile>	take spec(s) from file
-<n>		print out <n> days starting with the specified one
-n <name(s)>	holiday names to lookup
-db <dbfile>	database file
-z[=<b>]	use GMT: 0=no, 1=yes (default is NO)
-y <year>	alternate year of holiday
-V		print command version to standard-error and then exit

