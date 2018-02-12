DIALERPROG

This is a dynamically loaded module that implements a loadable "dialer" for the
SYSTEM(network) object. It is both configured to be used and optionally somewhat
configured itself through entries in (so-called) "system" files. The "system"
files describe ways for client programs (using the SYSTEM object) to dial out to
servers without having to know the details (or even the connection) method used
to reach any given server.  This idea was inspired by the old UNIX UUCP
facility (where details of how to reach remote systems was essentially
hidden from clients).

This is a code module that dials out to a program (it makes a connection with a
program). This is a client-side thing. This is generally built as a dynamically
loadable shared object but it can also be built as a non-shareable relocatable
object, which will also be dynamically loaded.

Dialer entry synopsis:

<sys>	prog [-x <key>=<value>] [-df <def>] [-xf <xe>] 
		[-ef <errfile>] <progfile> <progargz> <progarg(s)>

where:

-x <key>=<val>
-df <def>
-xf <xe>
-ef <errfile>
<progfile>	file of program to execute
<progargz>	zeroth argument to program
<progargs>	arguments to program


Example:

ece	prog -df %R/etc/defs -xf %R/etc/xe /file/path/prog + -a '%a'

The following cookies are substituted for:

SYSNAME		OS system-name (SYSNAME)
RELEASE		OS release
ARCHITECTURE	OS architecture
ARCH		OS architecture (short for 'ARCHITECTURE')
MACHINE		OS machine
N		current node name
D		current domain name
H		current host name
R		current program root
U		current username
h		remote system (usually a host name or something similar)
s		remote service
a		service arguments
t		timeout in seconds

The following environment variables are exported to the program:

SYSNAME
RELEASE
ARCHITECTURE
MACHINE

USERNAME	current username
NODE		current node
DOMAIN		current domain
HOST		current host
DIALER_ROOT	dialer program root
DIALER_HOST	target system
DIALER_SVC	service
DIALER_SVCARGS	service arguments

= TESTING

To test, make the test program named 'testdialprog'.  It is mostly
contained within the file 'main.c' (otherwise unused by the dialer
itself).

Synopsis:
$ testdialprog <host> <programpath> <arg0> <arg1> <arg2>

