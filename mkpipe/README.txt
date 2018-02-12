MKPIPE

This program will create named FIFO-special files in the filesystem. This
capability (named FIFO special files) was just introduced recently with System V
Release 2. Only one other person that I know of has taken advantage of this new
capability so far (and he is a contractor). We (Bell Labs guys) should be
ashamed of ourselves for not thinking this idea through and taking advantage of
it more than we have so far.

My pet idea about providing for read and write attention on each end of a pipe
did not get implemented. Further, the named FIFO idea as implemented is really a
degenerate single directional pipe with both ends appearing at the same i-node
in the filesystem.

