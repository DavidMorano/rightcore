PCPIO (POSIX Copy In Copy Out)

This program provides the POSIX compatible version of CPIO for several
OS-machine platforms. The POSIX version, besides being standardized, was the
version that was shipped with UNIX System V Release 3 and earlier (yes, the Bell
Labs got their thing to be the standard). However, it should be noted that the
version of CPIO shipped with UNIX System V Release 4 (and possibly following)
does not by default conform to the POSIX standard! Why? Because in System V
Release 4 they had bigger device numbers and needed a way to handle that. The
POSIX standard did not accomodate those larger device numbers.

Several OSes are sort of in limbo land between UNIX System V Release 3 and UNIX
System V Release 4. They need to be examined for whether they support the POSIX
(UNIX System V Release 3) standard or the newer UNIX System V Release 4
standard. Some of these systems are handled by this program.

