NOTIFIER

This little program sends a terminal notice out according to the specified
arguments. It is used mainly from CGI web-server programs.

Synopsis:
$ notifier <type> <user> [-f <from-agent>] [-m <msg>] [-V]

Arguments:
<type>		type of notification
<user>		some argument for notification
-f <from-agent>	indiction where notification is from and originating
		agent (if available)
-m <msg>	message text
-V		print program version to standard-error and then exit

