PCSGETMAIL

This program is used to get new mail from one or more user mail-spool
repositories.  By default all mail spool areas found in the system are searched
for new mail for each of the mail-users specified.  Both mail spool areas and
mail-users can be specified through environment variables.  The destination for
new mail is concatenated and placed into a single mailbox, either specified as
an argument or through one or more environment variables.  By default, if the
MBOX environment variable is found, it is used as the target for all new mail.
Further, a special daemon mode is specified with the 'fd' argument, giving a
file-descriptor (instead of a mailbox) for where all retrieved mail should be
placed.

Synopsis:
$ pcsgetmail [<mailuser(s)>] [-u <mailuser(s)>] [-m <mailbox>] [-t <timeout>]
	[-r <reportfile>] [-j <jobid>] [-fd <fd>] [-n] [-of <ofile>]
	[-md <dir(s)>] [-o <opt(s)>] [-V]

Arguments:
<mailuser(s)>		mail-users
-u <mailusers(s)>	alternate mail user(s)
-m <mailbox>		target mailbox to receive new mail (default 'new')
-t <timeout>		timeout during acquisition
-r <reportfile>		file to report transfer results
-j <jobid>		JOB-ID used during acquistion
-fd <fd>		file descriptor to receive new mail
-n			do lock lock destination mailbox
-of <ofile>		output file
-md <dir(s)>		mail-spool directories
-o <opt(s)>		options: nodel
-V			print program version to STDERR and then exit

