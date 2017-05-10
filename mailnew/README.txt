MAILNEW

This command examines the new mail for a user.

Synopsis:
$ mailnew [-<n>] [<user(s)>] [-af <afile>] [-o <opt(s)>] [-md <maildir(s)>] 
	[-lf <log>] [-V]

Arguments:
-<n>		maximum number of entries to print
<user(s)>	user(s) to examine for new mail
-af <afile>	argument-list file
-o <opt(s)>	options: sort[={for|rev|0|1}], date[=long]
-md <dir(s)>	mail spool directory(s)
-lf <log>	log-file
-V		print command version to standard-error and then exit

