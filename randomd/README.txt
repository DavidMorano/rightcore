RANDOMD ** unfinished **

This is a daemon program that creates random data and writes it to the device
file '/dev/random' (and 'dev/urandom'). The device file is a FIFO type file that
is set for reading by anyone. The file is generally only writable by the owner
of this daemon program.

Programs wishing to get some random data can open '/dev/random' for reading and
read as much as they want from there.

