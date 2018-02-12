AIRPORTS

This daemon program monitors Apple AirPort wireless WiFi base stations for
logging messages.

Synopsis:
$ airports <syslogfile> -dd <datadir>

Arguments:
<syslogfile>		the system log file to monitor
<datadir>		the directory to store the data

Configuration File:
# key			value
listen			tcp	inet/*/1051
listen			uss	/tmp/airports

