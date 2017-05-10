REX

= Dave Morano, 96/11/01
version '0'

This is a program packaged up version of the old 'rexec' capability
which was originally used in the old RSLOW program.  (Newer
RSLOW programs abandoned any explicit 'rexec' usuage in favor of
'rfile' and 'rex' direct subroutines.)


= Dave Morano, 96/12/12
version '0a'

1) This version was modified to take a username and password from a file.
   This was done so that they would not be logged in any command line
   accounting file !


= Dave Morano, 97/01/10
version '1a'

1) This version has better (hopefully much better) handling of the
   various I/O plumbing to and from the remote connection.

2) This version uses the new 'rexecl' subroutine to connect
   to the server.  This subroutine version is MT safe and
   will not print out stupid error messages.


= Dave Morano, 97/01/21
version '1b'

1) This version is an attempt to read the user's 'netrc' and
   the system-wide 'netrc' files.

2) Added :

	a) a sanity check option where a "ping" is periodically
	   sent out to the remote host

	b) authorization "levels" were added so that the user can restrict 
	   the connection attempts to certain usernames and passwords

3) I changed the name of the whole program to REX to completely
   eliminate the conflict with the old AT&T "REXEC" program.
   When this program was originally started eons ago, long before
   it evolved into its present form, the AT&T REXEC (Remote EXECution)
   package was known about but thought to be lost forever.  When it
   showed up in recent years, the conflict in names was observed
   and annoying.  It is now over with !!!


= Dave Morano, 97/03/15
version '1c'

1) We reduced our dependency on using DNS.  (We experienced a case where
   the corporate DNS was out-of-date but the local data was OK and we
   were out-of-commission dues to the problem.  Now we used the local
   information if it is sufficient without going to the DNS.)

2) We added the feature of taking an IP address (in dotted decimal notation)
   directly on the command line as a host specification.


= Dave Morano, 97/05/27 (completed)
version '1d'

1) We added the feature of handling the transfer of environment variables
   in a way similar to AT&T REXEC and Morano RSHE !

2) We made the LOGFILE program invocation option key now accept
   either an explicit value with and equals ('=') sign or
   a value following without (this is new) an equals sign character.

3) We fixed a problem where the COMPLEX (environment) program mode was
   not be entered if only a change of directory specifier was given
   at program invocation.

4) We added support for the environment variable 'NETRC'.

5) I broke and then fixed a problem with not entering COMPLEX program
   mode for when the entire environment is to be transferred to the
   remote machine.


= Dave Morano, 97/05/28
version '1e'

1) This is an attempt to carve out the data transfer part of the code
   into its own section so that it can be more easily probed for
   bugs (which may be there!).



