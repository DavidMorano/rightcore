MSINFO

This program prints out information about the nodes in the current cluster.

Synopsis:
$ msinfo [<node(s)>] [-s <sortkey>] [-r] [-nh] [-<n>] [-db <file>] [-V]

Arguments:
<node(s)>	a node in the cluster to print information on
				-	specifies the current node
				+	specifies the best node
-s <sortkey>	an optional field to sort output on; one of:
				utime stime dtime la1m la5min la15min speed
-r		reverse the sense of the sort
-nh		don't print the header
-<n>		limit output to the first <n> nodes
-db <file>	machine status database file
-V		print command version to standard-error and then exit

