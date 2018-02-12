EMA

This program extracts EMAs from message headers.

Synopsis:
$ ema [<file(s)>] [-af <afile>] [-h <header(s)>] [-s <subpart(s)>] [-n] [-r]
	[-o <opt(s)>] [-V]

Arguments:
<file(s)>	files to read email addresses from
-af <afile>	argument-list file
-h <header(s)>	which headers to extract from
-s <subpart(s)>	which subparts of the email address to display
			address, route, comment, best, any, original
-n		print out the number of addreses extracted
-r		short-form for specifying recipient headers (to,cc,bcc)
-o <opt(s)>	option is one of:
			expand[={0|1}] (default is 'expand=1')
			list[={0|1}]
			info[={0|1}]
-V		print program version to standard-error and then exit

Examples:
$ ema -h to < file.msg
would extrat all of the EMAs associated with "to" headers (the default).
$ ema -h to,cc < file.msg
would extrat all of the EMAs associated with "to" and "cc" headers.
The following will extract several subparts of an address:
$ ema -s address,comment,route < file.msg

