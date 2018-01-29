TELNETD

This program is a Telnet server program. It is hacked from the Sun Microsystems
version (which has the 'telmod' hack in it). This particular version of a Telnet
server is used to get a freebee login (no login or password sequence) to the
user 'guest'. This is useful for allowing a special service to run as user guest
in a sandbox type of environment (on the server side).

Is this a security hole? Probably, but life is hard enough already without
looking for trouble where there probably isn't much to be found. The security
part is how safe the login profile for user guest is and whether a malicious
user can break out of it and get a login shell prompt!?

