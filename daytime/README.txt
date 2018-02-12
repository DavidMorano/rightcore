DAYTIME

This program is really an INET server. It just prints the time of day in GMT. It
is meant to be a server on INET service port 'daytime'.

As an extra bonus, if optional positional arguments are supplied, they will be
contacted for the 'daytime' service and all such contacted hosts will have their
daytimes printed out.

Synopsis:
$ daytime [<hostname(s)>] [-d <dialer>[:<port>]] [-s <service>] [-x]
	[-f <af>] [-wl <n>] [-V]

Arguments:
<hostname(s)>		hostname to contact
-d <dialer>[:<port>]	dialer and optional port to use:
				udp, tcp, tcpmux, tcpnls, uss, ussmux
-s <service>		alternative service to use (default is 'daytime')
-x			do not interpret results (print anything "as is")
-f <af>			INET address family:
				unix, inet, inet4, inet6
-wl <val>		write-length (for UDP requests)
-V			print program version to standard-error and then exit

