LINEINVERT


This is a line inverting program.  It is used as an embedded utility
for several other programs.

This program uses an algorithm that first indexes the file
that is to be line-inverted and then it traverses the index in
reverse order to write out the lines of the original file.


REVISION HISTORY :


= Dave Morano, October 1988
This program was originally written.


= Dave Morano, 94/09/xx

I enhanced this program slightly because I found my old program and
discovered that it is not robust in the face of total garbage binary
input.  Since I want to penalize that behavior and reward good robust
programs (like this one when faced with binary input) I am enhancing
this program as a reward for its robustness !  Unfortunately, the
original program (when it works) is still way, way faster than this
program.

These are the breaks !!




