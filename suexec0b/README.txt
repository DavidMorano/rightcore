SUEXEC

The main use of this program is to run an interpreter file with SETUIDs. Note
that some Bourne shell interpreter files will not run set-UID or set-GID since
they tend to reset the effective UIDs to the real IDs.

This program is used much in these SGS packages to change the name under which a
program executes. It is similar to the 'suid_exec' program that is available
with KSH88.

Synopsis:
$ suexec program_path [-V] [arg0 arg1 arg2 ...]

Examples:
$ suexec uupoll.ksh uupoll sparc1 rcb

FUTURE POSSIBLE WORK:

Options:

	-u username	run program with specified effective username
	-g groupname	run program with specified effective group
	-eu username	run with effective user
	-ru username	run with real user
	-C configfile
	-ROOT programroot

Program Configuration File:

key		user		allowed EUIDs
--------------------------------------------------------------

user		dam		root,special,uucp
user		admin		special

Create a 'user' file, modeled after the 'group' file, as:

username:passwd:user1,user2,user2,user2,...

This file indicates which user, 'user1', 'user2', etc, can run 
SUID programs as 'username'.

Example:
root::dam,admin,adm,uucp
uucp::dam,admin

