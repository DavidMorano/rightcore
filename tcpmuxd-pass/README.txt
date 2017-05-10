TCPMUXD-PASS

This little program passes a file descriptor (FD) from its standard input (by
default) or a specified given FD to the TCOMUXD server program.  A custom
program (this one) is needed to negotiate the inter-process communication
protocol for passing of file descriptors to the TCPMUXD server when it was not
previously configured to listen on a PASS file of some kind.

Note that this program cannot (currently) be used to pass a file descriptor to
a PASS file.  Configuring a PASS file for the TCPMUXD program is also possible
and that PASS file is just a plain file to accept passed FDs in the normal way
(there is no special protocol needed for that).

This program is only needed when a PASS file is not configured by the TCPMUXD
server.

Synopsis:

$ tcpmuxd-pass <file>

where:

<file>		is the pass-file

