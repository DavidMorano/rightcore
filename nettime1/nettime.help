NETTIME

This program can query the time of a remote machine and display the
time or, optionally, set the local machine time from this remote time.

A time server (for testing purposes) to use for setting local computer
time is:

	ntp1.ece.neu.edu

Synopsis:
$ nettime -s [-proto <proto>] [-f <af>] <server(s)> [...]

This form is used to set the UNIX® system time from the first server
that is listed above.

To simply query for the time from a server, use:

$ nettime <server(s)> [...] -p

the first server that responds is used and its time is printed out.
To print out all of the times from any responding servers, use:

$ nettime <servers(s)> [...] -a -p

For testing purposes, you can specify a timeout for contacting
a server with:

$ nettime <servers(s)> [...] -t <timeout> -p

