BIBLEVERSE

This program takes a query (invocation argument or on standard input)
and prints the corresponding Bible verse.  A verse query takes the form:
	<book>:<chapter>:<verse>
The <book> part of the specification should be numeric.

Synopsis:
$ bibleverse [<query(s)> ...] [-af <afile>] [-a] [-n <nverses>] [-w <cols>] 
	[-y <year>] [-t <qtype>] [-z[=<b>]]
	[-<n>] [-ndb <ndbname>] [-vdb <vdbname>] [-o <opt(s)>] [-V]

Arguments:
<query(s)>	a verse specification indicating a verse to print out
-af <afile>	file of argument queries
-a		print out the whole Bible (text filled)
-<n>		number of verses to print (default is one)
-n <nverses>	number of verses to print (default is one)
-w <cols>	output column width
-y <year>	default year to query for
-t <qtype>	query type: 'verse', 'mjd', 'day'
-z[=<b>]	use GMT: 0=no, 1=yes (default is NO)
-ndb <ndbname>	the bible-name DB-name
-vdb <vdbname>	the bible-verse DB-name
-o <opt(s)>	options: gmt[=<b>]
-V		print command version on standard-error and then exit

Notes:
Verse queries are of the form (whitespace allowed between tokens):
	<string(s)>[:]<c>:<v>
or
	<b>:<c>:<v>

where:
<string(s)>	are one or more alpha-strings
<b>		numeric book
<c>		numeric chapter
<v>		numeric verse

