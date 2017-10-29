LISTEN

This is the old SYSV listener program from Solaris.

* See update note at the end of this diatribe. *

There is a bug in Solaris where an incomming TLI connection on a TCP transport
will somehow close or otherwise error permanently when the client side does a
shutdown with SHUT_WR close after the connection is opened.  If the SHUT_WR is
done about one second after the connection is opened, nothing bad happens.  The
NLS request-response exchange seems to occur without incident though.

An exact FAIL scenario is (client side):

1) open TCP socket
2) send NLS request string
3) receive NLS response string
4) write some data (anything)
5) do the SHUT_WR
6) server side (NLPS_SERVER) says that it can't push the specified modules!!


Stupid Solaris!
Finally, did I already mention that Solaris is buggy??????????????????

= Updated note:

I fixed this problem and several other (all related) problems.  I substantially
rearranged code in the NLPS_SERVER program to push modules onto the connection
before returning the NLPS response code to the network initiator.


