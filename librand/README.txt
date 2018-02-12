LIBRAND

This is a hack version of the "old" AT&T random variable library named
'librand'. This contains the so called "truerand" functions. These functions
might have been considered pure crap if it wasn't for the fact that the people
over at AT&T make them (I'm a former AT&T Bell Laboratories researcher myself!).

The original version of this library could not be semantically used correctly in
multithreaded programs under the Solaris 2.x UNIX OS. There is a bug in Solaris
UNIX that prevents the use of 'getitimer(2)' from working properly in
multithreaded programs. Of course, I needed a multithreaded version of this old
library, hence the hack!

