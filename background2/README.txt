BACKGROUND

This is a SHELL builtin command. This command puts a program into the background
and detached from the SHELL that started it. The background program is monitored
until it exits and then an optional mail message with its STDOUT and STERR is
sent to the invoking user.

This command is both the client (program that initiates the background running
program) and the server (program that monitors the background running program).
The server is only spawned off if no existing server is found. Client and server
coordination is through a common program root.

Synopsis:
$ background [-a <arg0>] [-of <ofile>] [-ef <efile>] [-if <ifile>] [-nice <v>]
	<program> [<args>]

Arguments:
-ROOT <pr>	program root
-a <arg0>	optional arg0 for the program
-of <ofile>	optional output file for the program (mailed to user otherwise)
-ef <efile>	optional error file for the program (mailed to user otherwise)
-if <ifile>	optional input file for the program (none otherwise)
-nice <v>	nice value
<program>	program to execute
<arg(s)>	optional arguments to the program

