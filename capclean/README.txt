CAPCLEAN

This is a support utility program for CAP (the famous AppleShare file server
system)!

This program searches through directory hierarchies and removes resource forks
and finder information forks for files that have had their respective data forks
already deleted.

This program is used in conjunction with AppleTalk shared directories from UNIX.
AppleTalk shared directories store the finder information and the resource forks
of files in separate subdirectories of any directory containing a shared file
that is access. This program cleans up dangling file forks for otherwise removed
files.

