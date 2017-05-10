FT

This program performs a file transfer (FT).
This program uses the FTP protocol to transfer files in a similar
way as the "rcp" program does.

It is envisioned that this program will perform something like:

$ ft file1 file2 machine:dir1/dir2

$ ft file1 machine:dir1/file2

$ ft -N netrc1:netrc2 file1 machine:dir1/file2

$ ft -N netrc1:netrc2 -p password file1 machine:dir1/file2

$ ft -N netrc1:netrc2 -p password file1 user@machine:dir1/file2

$ ft -A authlevel -N netrc1:netrc2 -p password file1 user@machine:dir1/file2

$ ft mach1:file1 mach2:file2 machine:dir1/file3




= version 0a, David Morano, 98/07/10

This is the first version of this program totally.
Although one would think that a program like this should
have been around already for a long time, it has not been!



