TESTLK

This does not test the LKFILE object! It tests whether file system record locks
work! Oh, they are supposed to work everywhere, you say? If only!

It was suggested (numerously) that file system record locks DO NOT work on
the'tmpfs' type of file system. So far, it appears that record locks do indeed
work on 'tmpfs' file systems. Of course, continuous and further testing is
welcomed.

Synopsis:
$ testlk.x -v /var/run/testfile

