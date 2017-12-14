MUXSERVE

This program is a general multiplexing server that can be configured to provide
a variety of services. Generally, LOGIN should not be provided through this
server as only STDIN and STDOUT are mapped to the network Telnet connection.

Synopsis:
$ tcpmuxd [-R <pr>] [-d=<n>] [-pass[=<n>]] [<svc>] [-ra] [-V]

Arguments:
-R <pr>		use this specified program-root
-d=<n>		enter daemon-mode for the optionally specified period <n>
-pass[=<n>]	client-side FD (<n>) pass is requested (daemon-mode is ignored)
<svc>		optional service (daemon-mode is ignored)
-ra		reuse-address
-V		print the program version to standard-error and then exit

