TCPMUXD

This program is a TCPMUX server that can be configured to provide a variety of
services. 

Synopsis:
$ tcpmuxd [-R <pr>] [-d=<n>] [-pass[=<n>]] [<svc>] [-ra] [-V]

Arguments:
-R <pr>		use this specified program-root
-d=<n>		enter daemon-mode for the optionally specified period <n>
-pass[=<n>]	client-side FD (<n>) pass is requested (daemon-mode is ignored)
<svc>		optional service (daemon-mode is ignored)
-ra		reuse-address
-V		print the program version to standard-error and then exit

