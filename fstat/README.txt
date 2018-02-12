FSTAT

This program prints out the modification date of files in different
possible formats.

Synopsis:
$ fstat [<files(s)> ...] [-af <argfile>] [-f <format>] [-V]

Arguments:
<file(s)>	files to process
-af <argfile>	file of files to process
-f <format>	output format of modification date:
			dec, decimal, int (default)
			touch
			ttouch, tt, or toucht
			access
			info
			log
			logz
			std
-V		print program version to standard-error and then exit

