PCSJOBFLE

This program creates a job file (a file in the filesystem) for use by programs
to that send jobs to other machines. This program pretty much just calls the
'mkjobfile' subroutine and returns the resulting job file name to standard
output.

Synopsis:
$ pcsjobfile [-r] [-t <timeout>] [<jobdir>] [-mtg <mtgdate>] [-V]

Arguments:
<jobdir>	job directory
-t <timeout>	timeout
<mtgdate>	a meeting date
-V		print program version to standard-error and then exit

