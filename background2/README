BACKGROUND

This is a SHELL builtin command.  This command puts a program into the
background and detached from the SHELL that started it.  The background
program is monitored until it exits and then an optional mail message
with its STDOUT and STERR is sent to the invoking user.

This command is both the client (program that initiates the background
running program) and the server (program that monitors the background
running program).  The server is only spawned off if no existing server is
found.  Client and server coordination is through a common program root.

Synopsis:

$ background [-a arg0] [-of outfile] [-ef errfile] [-if infile] [-nice v]
	program [args]

where:

-ROOT progroot	program root
-a arg0		optional arg0 for the program
-of outfile	optional output file for the program (mailed to user otherwise)
-ef errtfile	optional error file for the program (mailed to user otherwise)
-if infile	optional input file for the program (none otherwise)
-nice v		nice value
program		program to execute
args		optional arguments to the program

