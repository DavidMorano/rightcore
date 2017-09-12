BIBLESET

This program typesets the Bible (or any text in appropriate database form)..

Synopsis:
$ bibleset <file(s)> [-ff <fontfamily>] [-ps <ps>[:<vs>] [-vs <vs>] 
	[-vzle <w>[:<p>]] [-front <front>] [-ibz] [-back <b>]
	[-ndb <ndb>] [-mdb <wdb>] [-cover <cover>] [-tc[=<n>]] 
	[-V]

Arguments:
<file(s)>		text file(s) to process
-ff <fontfamily>	is one of: T, P, B, N, H, R, A, C
-front <front>		include front-matter from file <front>
-ndb <ndb>		bible book-name DB
-wdb <wdb>		bible meta-word DB
-cover <cover>		cover-page file
-ibz			ignore book number zero (default)
-tc[=<n>]		switch for index page (table of contents) at rear
-vzlw <w>[:<p>]		verse-zero line width <w> (and optional percent <p>)
-V			print program version to standard-error and then exit

