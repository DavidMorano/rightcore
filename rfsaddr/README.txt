RFSADDR

I lost the original one so this is a look-alike version.  I don't even remember
what the arguments were or how the first one worked !  This version looks more
like the later 'hostrfs' program that was distributed with the AT&T REXEC
package.  This one is a bit more flexible since it can create TLI addresses for
the UNIX address space as well as the INET address space.

Anyway, this one is just a rip off of the newer program 'tliaddr'.

Synopsis :

$ rfsaddr [[hostname] service] [-f family]


The default for the address family is 'inet'.
Note the following example :

$
$ rfsaddr anyhost listen
\x00020ACE000000000000000000000000
$
$

Enjoy,
Dave Morano

