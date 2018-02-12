%{SS}

This command is used to turn on or off the reception of terminal messages. 
This command also prints out session messages for the KSH shell (when
run as a built-in command).

Synopsis:
$ ${S} [-n|-y] [-m[=<n>]] [-b[={y|n}]] [-v[=<n>]] [-dev <termdev>] 
		-<n> [-o <opt(s)>] [-w <width>] [<fromuser(s)>] 
		[-T <termtype>] [-t <mtype(s)>] [-Q] [-V]

Arguments:
-y		turn terminal messaging ON
-n		turn terminal messaging OFF
-b[={y|n}]	turn BIFFing on or off (default ON)
-m[=<n>]	print session messages, optionally only <n> earliest		
-<n>		only print this many messages (the earliest)
-o <opt(s)>	options: sort=<type>
-w <width>	line-width for formatting messages
<fromuser(s)>	restrict messages to those from the given user(s)
-t <mtype(s)>	restrict messages to those of type <mtype(s)>
-dev <termdev>	terminal device
-v[=<n>]	set verbosity level (default is 1)
-T <termtype>	terminal type
-Q		do not complain if there is no login terminal!
-V		print command version to standard-error and then exit

Exitcodes:
0		messages are ON
1		messages are OFF
other		error

