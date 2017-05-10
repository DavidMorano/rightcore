
This directory contains the source code for the PCS BBPOST command.
We are up to version "2b" as of this writing.

= Dave Morano, 98/06/02
  I fixed the flags in the loop to get the newsgroup.
  There was a problem where if a bad newsgroup was given,
  we would still exit this loop and into the next
  (which gets the subject).



