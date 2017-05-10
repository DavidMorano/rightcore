

This version is the same as "0f" but has the 'system(3)' call in
'editit' changed to a 'bopencmd(bio)' instead.  This is to get around
some sort of problem when running on SYSV (the program was compiled on
BSD) where the 'ed' program is presummed to be in '/usr/ucb' (a
non-existent directory on SYSV).

Dave Morano, 95/10/30

