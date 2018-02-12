BBNEWS

This is the (old) famous BB program. This program is used to read articles out
of the BBnews subsystem of the Personal Communication Services (PCS) system. It
might be pretty junky (I did not start it from scratch myself !) but sometimes
it gets the JOB done when no other programs can !

Synopsis:
$ bbnews [-a|-o] [-e] [-sort={compose|post|arrive|modify|spool}] 
		[<newsgroup(s)>]

Arguments:
-a		all articles (old and new)
-o		old articles only
-e		every newsgroup
-sort=<type>	optional type of sort desired
newsgroups(s)	newsgroup(s) to examine

Caveats:
When using sorting modes other than spool (modify), the date stored in the user
currency file is the spool (modify) date rather than the date used for sorting.

Adding envelopes to articles when extraing to a mailbox:

Sometimes it might be desireable to add a new envelope to an article when it is
extracted into a mailbox file. This can be done with some command-line options
such as:

$ bb -mailbox <board> -envdate compose -envfrom <email>

