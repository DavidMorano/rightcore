MKREADABLE

This program will traverse directory hierarchies and change the modes on files
so that they are readable by "others." This is necessary (in a secure
environment) for backups to be taken over the network by the 'root' operator on
other machines.

Example:
$ mkreadable <dir1> <dir2>

would traverse directories 'dir1' and 'dir2' and make the files readable to
"others."

