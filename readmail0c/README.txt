
= Dave Morano, 96/05/23

VERSION 0a

This is the only one that works, including the
'system(3)' PATH messup problem.
This version also uses a program called 'rm-smail' as its
mail sending agent.


= Dave Morano, 96/06/18

VERSION 0b

1) I apply the standard 5 minute timeout on the mail spool lock
   so that old ones are removed.

2) I fixed what appeared to be a pretty bad bug when creating the users
   system delivery mailbox file name in the abscence of no MAIL variable.

3) The program will now log its usage in the PCS log directory
   under the log file name 'readmail'.

4) This version now uses a program named 'smail' to send messages.
   I think that 'smail' will stay around while the other names
   may transition.


= Dave Morano, 96/10/11

VERSION 0c

1) I fixed (enhanced -- whatever) a problem where it would complain
   about unknown mail options (specified in the environment variable).

2) I added the 'V' command to print out the program version.

3) The program will choose as a mailer whatever is in the "config"
   header include file as PROG_MAILER but modified according whether
   the READMAIL program is invoked as an "old", "no distinction",
   or "new" version.


