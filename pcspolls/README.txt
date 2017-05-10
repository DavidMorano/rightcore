PCSPOLLS

This directory contains the source code for some PCSPOLL loadable (poller)
objects.

There is no actual program to run here.  Rather, the poller loadable object
modules get executed indirectly as a result of executing other PCS programs
that are configured to call the poller (which in turn calls the poller
modules).

Some sample PCSPOLL objects are supplied.  These are:

+ loguser		provides a simple log of the user
+ pollprog		conditionally calls the PCSPOLL program

Enjoy.


