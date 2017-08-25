TEXTCLEAN

This program cleans up crap left over from (mostly) Micro$noft type programs.
Apparently, many M$ program leave wierdo and other weirdo DOS-like character
all over plain text files!  What is with that?

Synopsis:

$ textclean [<file(s)> ...] [-af <afile>] [-o <opt(s)>] [-mf <mfile>] [-V]

where:

<file(s)>	one or more files to clean up (they are concatenated
		by default)
-af <afile>	argument list-file
-o <opt(s)>	one or more of:
		inplace		write cleaned up file back to its original
		lower		change to lower case
		leading		remove leading white-space
		trailing	remove trailing white-space
		double		double space the lines
		half		half-space the lines (where they were double)
		oneblank	for a maximum of only one blank line
		mscrap		remove M$ crap!
		pad=<width>	pad w/ spaces out to <width>
		snug		remove white-space from between words
-mf <mfile>	specify an alternative M$ character-map file

