ECOVER

This program creates a cover-sheet for files that are generally to be
subsequently encrypted.

Synopsis:

$ ecover [{ -d | -e }] [<file>] [-m <message>] > [-of <ofile>] [-V]

where:

<file>		optional source file, otherwise STDIN is used
-e		perform encoding
-d		perform decoding
-m <message>	an optional message to be encoded as metadata
-of <ofile>	output file
-V		print program version to standard error and then exit

