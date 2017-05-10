DIALER_UUX

This is a code module that dials out to a program (it makes a connection
with a program).  This is a client-side thing.  This is generally built
as a dynamically loadable shared object but it can also be built as a
non-shareable relocatable object, which will also be dynamically loaded.

Dialer entry synopsis:

<sys>	uux [-R <pr>] [-g <grade>] [-u <username>] [-n] [-r] 
		[{ [[<node>!]<svc>] | [[<node>][[!]<svc>]] } [<args>]]

Example:

rc	uux rc!mailbridge dam



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

To test, make the test program named 'testdialuux'. It is mostly contained
within the file 'main.c' (otherwise unused by the dialer itself).

Synopsis:

$ testdialuux host programpath arg1 arg2



