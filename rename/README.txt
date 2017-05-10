RENAME

This program renames files (specified as operand arguments) as per the
specifications given.  It is like a bulk move operation and all target names
are in the current directory.

Synopsis:
$ rename { <base> | -b <base> } [-c <start>] [-p <prec>] 
	[-s <suf>] [-i <increment>] [<file(s) ...>] [-af <afile>] [-V]

Arguments:
<base>		filename base
-b <base>	filename base
-c <startcount>	start counting at this number (default 0)
-p <precision>	precision of the count
-s <suf>	specify suffix to add to created file names:
			<suf>	add specified string
			'+'	use as suffix the first one found
			'-'	use existing suffixes
			''	specify no suffix
-i <increment>	increment value to add for successive files (default 1)
<file(s)>	a file or more to be renamed
-af <afile>	take file names to be renamed from this file
-V		print command version to standard-error and then exit

