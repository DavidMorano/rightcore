VMAIL

This is the famous PCS mail program.  This is a screen oriented program
that is enhanced to use the additional features of the PCS system.

Synopsis:
$ vmail [=tmpdir <dir>] [-maildir <dir>] [-folderdir <dir>] [<mailbox>] 
	[-u <mailuser(s)>] [-o <option(s)>] [-L <lines>] [-M <mailer-program>]

Arguments:
-sl <scanspec>		specify scan configuration: <sv>:<sj>
				sv	- scan view lines
				sj	- scan jump lines
-tmpdir <dir>		alternative temporary directory (default is '/tmp')
-maildir <dir(s)>	alternative mail-spool directory to use
-folderdir <dir>	the user mail-folder directory (default 'mail')
<mailbox>		initial mailbox to open on start-up
-o <option(s)>		options (see below)
-u <mailuser(s)>	list of mailusers to assume
-L <lines>		the number of lines to use on the current terminal
-M <program>		a mailer program to use for sending mail

Options:
getmail[={0|1}]		turn retrieving spool-mail ON or OFF (the default
			is determined by the PCS administrator)

