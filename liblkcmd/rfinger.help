RFINGER

This is a little small network client program that queries a remote FINGER
server for some information.  It doesn't care what comes back (in accordance
with the RFC Finger protocol) but rather just displays it for the user.

This program also includes several dialer options to use to connect to the
remote client in addition to just plain-ole TCP.  Of course, you will need
servers that are capable of listening on some weirdo other thing besides a
basic TCP port if you want to play with the other dialers.

Synopsis:
$ rfinger user@host [...] [-af <afile>] [-d <dialer>[:<port>]] [-p <port>] 
	[-s <svc} [-f <af>] [-l] [-a <svcargs>] [-t <to>] [-o <opt(s)>] [-V]

Arguments:
<user>			username
<host>			hostname
-af <afile>		argument-list file
-d <dialer>[:<port>]	specifies an alternate dialer (and optional port)
-p <port>		specifies an alternate port
-s <svc>		specified an alternate service
-f <af>			protocol address-family
-l			specfied "long format"
-a <svcargs>		optional service arguments
-t <to>			time-out
-o <opt(s)>		options: termout[=<b>]
-V			print command version to standard-error and then exit

