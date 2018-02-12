UUPOLL

This program is used to start a UUCP poll to the specified machines given on the
program invocation.

Synopsis:
$ uupoll machine [machine(s) ...]

This program consists of two different files.

file		description
---------------------------------------------------

uupoll		outer program
uupoll.ksh	inner program

Install as follows:

$ cp -p uupoll.ksh uupoll ${LOCAL}/bin/
$ cd ${LOCAL}/bin
$ chown uucp uupoll.ksh
$ chmod u+s uupoll.ksh

