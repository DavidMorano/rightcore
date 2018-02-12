SNAME

Shell-NAME (SNAME) program.

This program is meant to be used in "interpreter" files when you want to
interpret SHELL language.

For example, if you put the following at the top of the interpreter
file (for example):

	#!/usr/add-on/local/bin/sname /bin/ksh

when the file is executed, it will end up with an ARGV set as:

	<filepathname> <filepathname> ARG1 [ARG2 ...]

The Shell program being executed by the shell itself (KSH in this case) will see
a fako ARGV (passed from the SHELL) set as :

	<filepathname> ARG1 [ARG2 ...]

