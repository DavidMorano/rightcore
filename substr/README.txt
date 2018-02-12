SUBSTR

This program does what SED and ED cannot do easily. It substitutes one weirdo
string for another.

It reads lines in a file (line by line) and checks for the first string given to
the program. If found, the second is substituted for it.

Synopsis:
$ substr s1 s2 [<file(s)>] [-af <argfile>] [-of <outfile>]

