FILEOLDER

This program tests if any file(s) are older than a specified amount.

Synopsis:
$ fileolder <file(s)> -<age> [-V]

Arguments:
<file(s)>	file(s) to test modification date of
-<age>		age of file to test against (like '10h')
-V		print program version on standard-error and then exit

Returns:
0		file is older
!=0		file is not older

