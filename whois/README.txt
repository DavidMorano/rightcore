WHOIS

This is a little WHOIS client program. The beauty of this over the regular
supplied versions is that this version strips off the stupid extra carriage
return characters that M$ servers put onto the WHOIS output. The alternative was
to get rid of all M$ servers in the world but we couldn't get everyone to agree
on that (sigh).

Synopsis:
$ whois [-h <host>] [-p <port>] <name(s)> [-af <afile>] [-t <to>] [-V]

Arguments:
-h <host>	use this as an alternative WHOIS host
-p <port>	use this as an alternative WHOIS host
<name(s)>	name(s) to lookup in the database
-af <afile>	argument-list file
-t <to>		time-out
-V		print program version to standard-error and then exit

