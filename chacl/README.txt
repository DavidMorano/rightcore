CHACL

This command changes ACLs on files and directories.

Synopsis:
$ chacl [-r] [-c] <acl(s)> [<file(s)> ...] [-af <afile>] [-f] 
	[-s <suf(s)>] [-t <type(s)>] [-epf <file>] [-cu <user>] [-cg <group>]
	[-d] [-min[=<b>]] [-max[=<b>]] [-mm[=<b>]] [-of <ofile>] [-V]

Arguments:
-r		recurse into subdirectories
-c		continue on errors (rather than stopping)
-f		follow symbolic links
<acl(s)>	the ACLs (one "word" argument, comma separated)
<file(s)>	the files or directories
-af <afile>	more file (or directory) arguments in this file
-s <suffix(es)>	process only these file suffixes
-t <type(s)>	process only these file types (f, d, c, b, p, s)
-u <user>	process only these file(s) owned by <user>
-epf <exfile>	file of file-name pattern-strings to exclude
-cu <user>	change the ownership of the file to <user>
-cg <group>	change the group of the file to <group>
-d		delete empty ACLs
-min		perform "minimum" permissions operations
-max		perform "maximum" permissions operations
-mm		perform both "minimum" and "maximum" permissions operations
-mc		mask calculation -- recalculate the mask permissions
-of <ofile>	output file
-V		print command version to standard-error and then exit

ACLs (for CHACL) look like:
u[=user]{+|-}[rwx]
g[=group]{+|-}[rwx]
o{+|-}[rwx]
du[=user]{+|-}[rwx]
dg[=group]{+|-}[rwx]
dm{+|-}[rwx]
do{+|-}[rwx]

