DIALERUSS

This is a dynamically loaded module that implements a loadable "dialer" for the
SYSTEM(network) object. It is both configured to be used and optionally somewhat
configured itself through entries in (so-called) "system" files. The "system"
files describe ways for client programs (using the SYSTEM object) to dial out to
servers without having to know the details (or even the connection) method used
to reach any given server.  This idea was inspired by the old UNIX UUCP
facility (where details of how to reach remote systems was essentially
hidden from clients).

This is code module that dials out on a USS connection. This is a client-side
thing. This is generally built as a dynamically loadable shared object but it
can also be built as a relocatable object, which will also be dynamically
loaded.

