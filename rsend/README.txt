RSEND

This program sends one or more files to a remote machine. The directory on the
remote machine is by default the RJE directory of the recipient (who can be
specified). Other target directories can be specified explicitly also.

Synopsis:
$ rsend [files(s) ...] [-d targetdir] [-u username] [-f filename] [-V]

Arguments:
<files(s)>	are files to be transmitted
-d <targetdir>	optional alternate target directory
-u <username>	optional alternate username (default current user)
-f <filename>	filename of target when standard input is used
-V		print program version to standard-error and then exit

