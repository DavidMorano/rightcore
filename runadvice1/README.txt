RUNADVICE

This is a program that runs several simulations simultaneously (in parallel)
across a distributed set of computers. This is primarily used for ADVICE
simulations but it can be used somewhat for other types of simulations also. It
automatically spawns simulations spanning the various dimensions of the design
space parameters specified (either explicitly or through the various
configuration files). Simulations are spawned dynamically based on a combination
of factors on the set of networked computers in the execution pool.

= Dave Morano, 1996-04-02
version 0a

This version was just finished and was made from the '0' version.
I do not remember what the modifications were except that I think
that this is much faster although that really doesn't matter.

= Dave Morano, 1996-06-04
version 1

This version was just finished and was made from the '0a' version.
This version added the ability to run multiple ADVICE runs simultaneously
and to also load balance the various runs accross a pool of specified
machines.

