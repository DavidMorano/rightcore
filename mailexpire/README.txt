MAILEXPIRE

This command expires messages in a mail mailbox.

Synopsis:
$ mailexpire [<mailbox(es)>[=<age>]] [-af <afile>] [-t <age>] 
		[-md <maildir(s)>] [-u <user(s)>] [-a] [-V]

Arguments:
<mailbox(es)>[=<age>]	mailbox(es) to process w/ optional age <age>
-af <afile>		argument-list file
-t <age>		age over which messages are expired
-md <maildir(s)>	mail directories
-u <user(s)>		users to operate on (default self)
-a			operate on all users
-V			print command version to standard-error and then exit

