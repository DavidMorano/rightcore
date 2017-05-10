
This is a daemon, similar to 'pcnfsd', which handles PC authentication
and file locking requests from clients.  It can be run along with
NFSD, NIS, PCNFSD, and LOCKD.  Clients' requests can use any one of these
for serving specific needs.  This daemon is usually most needed when
a file server does not have one or more of : NIS, PCNFSD, or LOCKD.
This daemon does not replace NFSD.  The NFSD daemon is always needed
for file like client requests.

Dave Morano

