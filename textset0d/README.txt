TEXTSET

This program formats one or more input text files into an output TROFF source
language file.

Synopsis:

$ textset [-l <lines>] [-<lines>] [-of <outfile>] [<file(s)> [...]]
	[-f <font>] [-o <offset>] [-b <toplines>] [-DV]
	[-p <point_size>] [-v <vertical_space>]

where:

-<lines>		number of lines per page
-l <lines>		number of lines per page
<file(s)>		file(s) to process
-f <font>		TROFF type font specification
-b <toplines>		blanks lines at the top of each page
-fo <offset>		left margin shift offset
-offset <offset>
-of <outfile>		output file
-D			debugging flag
-V			print program version to standard-error and then exit

