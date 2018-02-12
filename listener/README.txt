LISTENER

This little program listens on a single socket address and spawns the specified
program when a connection comes in on that.

Synopsis:
$ LISTNER_ADDR=<tliadda>
$ LISTENER_PROGRAM=<program_to_spawn>
$ export LISTENER_ADDR LISTENER_PROGRAM
$ listener

Arguments:
tliaddr		a TLI address in HEX
program		path to program to run

