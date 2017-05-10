LIBB


This is the "Basic I/O" library.

To make:

$ makeit install


This library is very similar to the standard UNIX "stdio" library but
is optimized differently for different needs.  This library is
optimized well for more general file operations mode akin to those
available with the UNIX I/O system calls but also has support for the
string and buffer formating that "stdio" has as well.  We also support
a number of related UNIX utility like functions such as 'fstat',
'fcntl', and 'lockf' like functions directly in the library API.

This library was derived from a collection of previous "stdio" like
functions that I originally wrote (circa 1984) for use solely on
embedded circuit packs.  These became so popular ! :-) that I wanted
the same sorts of functions on UNIX !  So I collected the routines and
put together this library to that end.  This library continued to be
use on embedded circuit packs.  Most all of the UNIX system calls are
already supported on the OSes that I had on the circuit packs (most of
them variants of each other).  This library could also sit atop of a
couple tasking OSes that I put together for the BellMac 32 processor
environment.

Dave Morano, March 1986



REVISION HISTORY



= 99/01/10, Dave Morano

I updated a few of the subroutines to do memory mapped I/O.
It turned out to be a big waste since memory mapped I/O on
Solaris is pretty significantly slower (more than 20 % slower)
than regular I/O.  This serves to indicate at least two things :

	1) Leave old programs alone (or in this case, old libraries alone).  
	   They work well enough as they are !
           Don't you have better things to do with your time ?

	2) Somehow Solaris 2.5.1 performs the more complicated
           super-operation of internally mapping files and copying
	   their data to user's address space FASTER than a user
	   can simply map the same file !!  Please tell me how Solaris 
	   does this if you can !


= 00/05/03, Dave Morano

I changed references to the return codes 'BR_' to 'SR_' instead.
This is more in line with where the future of this library may be
headed (sitting on top of 'vs' that is) !  The 'BR_' codes are
now aliased to the 'SR_' return codes.  Some 'BR_' codes
have changed.  I hope that the impact is minimal !!



