PCSNAME

This program prints out the current PCS-name for a specified user.
The program can also set the PCS-name for the current user.

Synopsis:

$ pcsname [-u <username>] [-s <name>] [-f] [-V]

where:

-u <username>	retrience (or set) specified username
-s <name>	sets the PCS-name to <name>
-f		operate on the full-name rather than the regular name
-V		print the program version to standard error and then exit

Usually <name> should be quoted so that it is a single argument to
the program.

Example:

$ pcsname -s "John A. Smith"

