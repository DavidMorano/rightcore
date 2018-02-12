%{SS}

This is the PCS program to post articles to the PCS newsgroups.  This is the
raw interface to the newsgroup system.  The BBPOST program (and possibly other
programs) call this program to do the actual work of posting the article in the
newsgroup area and updating any databases that may need to be updated.

Synopsis:
$ %{S} [<newsgroup(s)> ...] [-af <afile>] [-f <from>] [-s <subject>] 
	[-e <datespec>] [-et <ndatespec>] [-R <pr>] [-N <newsdir>] 
	[-maint] [-expire] [-Q] [-V]

Arguments:
<newsgroup(s)>	the newsgroups that the article is to be spooled on
-af <afile>	argument-list file
-f <from>	specify the FROM-address of sender
-s <subject>	specify a subject
-e <datespec>	specify an expiration date (MMDDhhmm[YY])
-et <ndatespec>	specify an expiration date ([[CC]YY]MMDDhhmm[.ss])
-R <pr>		set program root directory
-N <newsdir>	the directory where the news articles will be spooled
-maint[=<int>]	ADM: perform maintenance w/ optional expiration interval
-expire		ADM: remove articles that have expired
-Q		be quiet about errors or newsgroups not found
-V		print program version to standard-error and then exit

