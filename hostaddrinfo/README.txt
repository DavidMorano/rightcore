HOSTADDRINFO

This program retreives INET address information for the specified
names (hostnames) and displays various elements of that information
to standard-output.

Synopsis:
$ hostaddrinfo [<name(s)> ...] [-af <afile>] [-q] [-f <af>] [-ao] 
	[-o <opt(s)>] [-V]

Arguments:
<name(s)>	hostname(s) to retrieve information for
-af <afile>	file of hostname(s) to retrieve information for
-q		do not print anything, rather just provide an exit code
-f <af>		only consider this address-family
-ao[=<b>]	address-only mode
-o <opt(s)>	option(s): 
-V		print command version to standard-error and then exit

