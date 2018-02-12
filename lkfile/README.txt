LKFILE

This program is used for creating lock files in the filesystem. Only one program
can successfully create a lock file so these files can serve as mutual exclusion
semaphores.

Synopsis:
$ lkfile [file(s) ...] [options]

options			description
-------------------------------------

-af <argfile>		take names of lock files from given file
-t <timeout>		use <timeout> as a time out for acquiring locks
-r <multiplier>		use the given multiplier for removing files
-V			print program version then exit
-HELP			print the help file then exit

