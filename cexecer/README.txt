CEXECER

This small program is the server side for making cluster "exec" calls.
It reads its standard input for the information about a program (lcated
in the cluster) to 'exec'.  It passes both its standard input, output,
and error file descriptors to the new program.

Generally, there are no options (except during debugging).

The program root for the program is retrieved from the "other" side!

This program also assumes little or no environment.

Synopsis:
$ /usr/add-on/local/lib/cexec/cexecer

