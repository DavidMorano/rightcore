DIALER_UDP

This is code module that dials out on a UDP connection.  This is a client-side
thing.  This is generally built as a dynamically loadable shared object but it
can also be built as a relocatable object, which will also be dynamically
loaded.

The shared object is given the filename 'udpfamily.so' and a symbolic link
should be made from this to 'udp.so'.

Non-shared objects can be used as the final module also, except that they do
not get shared on being loaded as shared objects do.


