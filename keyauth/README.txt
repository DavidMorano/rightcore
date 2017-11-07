KEYAUTH

This program authorizes a user for secure ONC services by decrypting the ONC
private key of the user and giving it to the key-server.

Synopsis:
$ keyauth [<username>] [-af <afile>] [-n] [-v[=<n>]] [-V]

Arguments:
<username>	username to use for decrypting private key
-af <afile>	argument-list file (of a username)
-n		do not do the decryption (just validate)
-v[=<n>]	set verbose-level
-V		print the program version to STDERR and then exit

