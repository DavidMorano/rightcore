MKINDEX

This program reads key-file information and creates the required index files
used in making queries. The index files created are:

	<idxname>.post
	<idxname>.names

Synopsis:

$ mkindex [-idx <name>] [<keyfile(s)>] [-af argfile] [-a] [-V]

where:

-idx <idxname>	name of the index
<keyfile(s)>	key-file(s) to index
-af <argfile>	argument-list file of key-file(s) to index
-a		append mode
-V		print program version to standard-error and then exit

