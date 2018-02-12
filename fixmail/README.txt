FIXMAIL

This program was used to debug the version of SENDMAIL (an old version) that was
distributed with the TCP/IP package on the AT&T 3B2 computers. That version of
SENDMAIL was buggy to the point of core-dumping and causing '/bin/mail' to core
dump also. They never found the problem, but I did with the help of this
program. The bug was with the SENDMAIL program's handling of environment
variables. I think that if there were too many environment variables in the
process environment, the SENDMAIL program would not properly pass down a valid't
constructed environment array (of course this all happens through the kernel
itself) but somehow the environment variables passed down were messed up and
this caused '/bin/mail' to core dump. This program was placed between SENDMAIL
and '/bin/mail' to clean up the environment variables passed down.

