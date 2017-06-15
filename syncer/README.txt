SYNCER

This command performs a synchronization of all of the file-systems. By default,
this occurs partly synchronously (see |sync(2)|). It can optionally also be done
in parallel with other tasks in the current session, or completely in the
background (detached from any session whatsoever). If done in parallel and the
current session is ended, the session will not actually exit until the
synchronization is completely taken up by the operation system.

Synopsis:
$ syncer [-b] [-o <opt(s)>] [-V]

Arguments:
-b		perform the synchronization in the background
-o <opt(s)>	option(s):
			parallel[=<b>] ­ perform in parallel
			background[=<b>] ­ perform in background
-V		print command version to standard-error and then exit

