IMAIL

This is a KSH shell built-in command. It injects a mail message (in mail-message
format) on STDIN into the mail network. By default mail recipients are taken
from the given mail message itself.

Synopsis:
$ imail [<recip(s)>] [-af <afile>] [-t[=<b>]] [-f <fromaddr>] 
		[-o <option(s)>] [-V]

Arguments:
<recip(s)>	optional recipients (in addition to ones inside the message);
		can be any of: <realname>, <groupspec>, <emailaddr>
-af <afile>	argument-list file
-t[=<b>]	take option (ON by default)
-f <fromaddr>	optional FROM address
-o <option(s)>	options:
			addfrom, addsender, log, folder=<dir>, deliver, copy,
			take, useclen, useclines, to, mailer, org[=<org>],
			subjrequired
-V		print command version to standard-error and then exit

Example recipients:
	d.a.morano, ¡tools, morano, david.a.morano@gmail.com

