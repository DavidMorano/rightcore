reply-to: dmorano@ece.neu.edu
bcc: dmorano@ece.neu.edu (verify test loop-back address)
From: (David Morano) morano@ele.uri.edu
To: (Professor Uht) uht@ele.uri.edu
cc: (Professor D. Kaeli) dkaeli@ac.upc.es 
cc: (Alireza Khalafi) akhalafi@ece.neu.edu
x-cc: (Tim Toolan) toolan@ele.uri.edu
x-cc: (Marcos de Alba) mdealba@ECE.NEU.EDU
x-cc: (Team Levo) levo <morano@computer.org>
x-cc: (Sean Langford) langfors@ele.uri.edu
x-cc: (Thomas Wenisch) iota@ele.uri.edu
x-cc: (Yuan Lin) <liny@ele.uri.edu>
x-cc: (Professor Kaeli) kaeli@ece.neu.edu
x-cc: (David Kaeli) dkaeli@ac.upc.es
x-cc: action@ele.uri.edu (URI ELE Action Help)
Subject: status

Professors,

Please find below an update on our simulation efforts.

Thanks,
Dave M.
=======================================================================
Status Report 02/04/26
Dave Morano


Here are the 10 benchmarks that we have so far, for our simulator :

+ bzip2
+ compress
+ crafty
+ gcc
+ go
+ gzip
+ ijpeg
+ mcf
+ parser
+ vortex


We have not done full simulation (with our new counters) on all of
these yet but we will be doing so in the coming week.

I have started to work on an idea that may provide better use of
speculative execution in the simulator.  It will try to reduce the
amount of speculative execution that occurs in the machine in the
earlier columns (not the commitment column).  I will provide more
information on this idea after I discuss it more with Alireza.

I am also working on how to gather additional event data in the
simulator.  These are primarily branch and memory things but we are
thinking about other events also (numbers of clocks waiting for things,
et cetera).  Right now our focus is on getting branch related events
and any associated DEE events that correspond to them (DEE paths
spawned for certain branches, or DEE path switches due to
mispredictions).  Besides branch related event data, we are planning on
a variety of memory related events and we may have some data on these
this coming week.  

I am also starting to code up a way to get some data on Hammock-style
branches.  I do not yet know how much detailed information we might be
able to get about Hammocks, but we think we can at least get
information on Simple Single-sides ones.

