GTAG

This program is a copletely new and fresh rewrite of the old TAG program from
AT&T Bell Laboratories. We miss that old program so we had to write one
ourselves. This program is essnetially a pre-filter that processes tag-citations
found in Memorandum-type (MM) TROFF documents. It was intended for MM-type TROFF
source text documents as input but can really be used on almost any TROFF
source. We handle some enhanced in-file syntax than the old TAG program did
also.

Synopsis:
$ gtag [<file(s)> ...] [-af <argfile>] [-V]

Arguments:
<file(s)>	file(s) to process
-af <argfile>	list of files to process
-V		print program version to standard-error and then exit

Usage in document:

.TAG ENCODER
This is a tag-citation \tag{ENCODER} right there!
Here is another \_ENCODER one (in an alternative form).

