TLIADDR

Create and print out a TLI address for a host.  The default without any
arguments at all is to print a TLI address for |anyhost:listen|.

Synopsis:

$ tliaddr [[<hostname>] <svcspec>] [-i <interface>[:<family>]] 
	[-f <family>] [-p <portspec>] [-V]

where:

<hostname>	name of host
<svcspec>	name of service (port for INET sockets)
-i <interface>	interface type ('raw' or 'socket')
-f <family>	protocol family ('unix', 'inet4', 'inet6')
-p <portspec>	protocol port (default for INET is 'listen')
-V		print program version to standard-error and then exit

