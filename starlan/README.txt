STARLAN

This program 'slsend' (STARLAN send) is used to copy files
to a the STARLAN which is connected to the originating host machine.
Currently, the only networking mechanism used is UUCP.

Synopsis:
$ slsend [-v] -[-b] [-d server] [-s sharename] [files ...]

Arguments:
-v		print out version to standard output
-b		do not convert the file to PC file format
-d server	copy file(s) to the STARLAN server 'server'
-s sharename	use STARLAN sharename 'sharename'

There are defaults established for both the server and the sharename.
Also, a default file conversion occurs that converts the file into PC
file format.

Please direct questions to Dave Morano, mtgzy!dam .

