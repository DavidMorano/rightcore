PROGSWITCH

This program provides a switching capability for finding programs to execute by
invocation name.

Synopsis:
$ <name> <args_for_real_program>

The program root for this program is found by one of the
environment variables (in order) :

1) PROGSWITCH_PROGRAMROOT
2) LOCAL

If neither of those are available, other alternatives are tried, finally ending
in a program-compiled default.

This program searches its program root for a file named 'progswitch.map'. That
file is a "KVS"-type file that contains entries indexed by the names of programs
that are to be switched.

If no available program is found in the program map, the existing execution PATH
is searched for a possible program.

An important part of a program like this is to avoid infinite loops in executing
ourselves repeatedly. Due to an administrative mistake (a VERY common event) we
can find an occurrance of ourselves and might execute if unless some
discriminating action is taken. We compare all candidate programs that we find
to see if they have matching device and inode numbers to our own execution file.
Matching executables are, of course, skipped. This provides a very excellent and
forgiving environment for administrative mistakes or intentions ! :-)

example program-map file
===============================

ls:
	${GNU}/bin/ls
	${AST}/bin/ls
	/bin/ls

other:
	whatever

