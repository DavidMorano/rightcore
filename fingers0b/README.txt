FINGERS

This program is a FINGER server that can be configured to provide a variety of
services. Note that only STDIN and STDOUT are mapped to the network connection.

Synopsis:
$ fingers [-R <pr>] [-d=<n>] [-pass[=<n>]] [<svc>] [-o <opt(s)>]
		[-ra] [-cmd <cmd>] [-V]

Arguments:
-R <pr>		use this specified program-root
-d[=<n>]	enter daemon-mode for the optionally specified period <n>
-pass[=<n>]	client-side FD (<n>) pass is requested (daemon-mode is ignored)
<svc>		optional service (daemon-mode is ignored)
-o <opt(s)>	option(s): pollint, ra, reuseaddr
-ra		reuse-address
-cmd <cmd>	enter client mode and execute command <cmd>
-V		print the program version to standard-error and then exit

