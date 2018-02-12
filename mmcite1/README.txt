MMCITE

This program is a pre-filter that processes citations found in Memorandum-type
(MM) TROFF documents.  It takes source MM-type TROFF source text documents as
input and produces output that has the citations annotated with their
bibliographical data.

Synopsis:
$ mmcite [<file(s)> ...] [-af <afile>] [-B <incdir(s)>] 
	[-p <bibfile(s)>] [-V]

Arguments:
<file(s)>		files to process
-af <afile>		list of files to process
-B <incdir(s)>		search directories for unrooted BIB files
-p <bibfiles(s)>	bibliographical files
-V			print program version to standard-error and then exit

Usage in document (note TeX style for in-line citation):

.BIB testreferm.rbd
This is a citation \cite{morano99helping} right there!

