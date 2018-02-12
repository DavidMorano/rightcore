MSGIDADM

This program is an administrative interface to the MSGID database.

Synopsis:
$ msgidadm [<recipient(s)> ...] [-s <sortkey>] [-db <database>] 
	[-m] [-a] [-<n>] [-m] [-td <displaykey>] [-r] [-h[=<b>]] [-nh] [-V]

Arguments:
<recipient(s)>		restrict listing to messages with given recipient(s)
-s <sortkey>		sort by time: 'update' (d), 'msg', or 'create'
-db <database>		use alternate database
-a			show all entries rather than default number
-<n>			show this number of entries (default 20)
-m			show MESSAGE-ID
-td <displaykey>	display time: 'mtime' (d), 'ctime', or 'utime'
-r			reverse sense of sorts
-h[=<b>]		header switch
-nh			specify no-header
-V			print program version to standard error and then exit

