QUOTE

This program takes a query (invocation argument or on standard input)
consisting of one or more words and prints out the corresponding quotes
that match.

Synopsis:
$ quote [<query> ...] [-af <argfile>] [-qd <dir(s)>] [-c <cat(s)>] 
	  [-w <cols>] [-o <opt(s)>] [[-V]

where:
<query>		a verse specification indicating a verse to print out
-af <argfile>	file containing queries
-qd <(dir(s)>	one or more quote directories
-c <cat(s)>	quote catalogues
-w <cols>	output column width
-o <option(s)>	options:
			interactive[=<b>]
			indent=<n>
			separate=<n>
-V		print the program version on standard-error and then exit

