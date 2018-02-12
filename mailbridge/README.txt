MAILBRIDGE

This progam bridges a mail message from one machine to another. This is pretty
simple and does not perform any queying. This means that messages transferred
through this program are potentially susceptible to loss. But since this program
us usually is in clustered or otherwise relatively connected environments, this
is not always a serious deficiency.

This program is similar to a cheapo SENDMAIL program except that it doesn't
queue up the mail message. Instead it sends it to some other program (usually
through over the network) to some other program.

Synopsis:
$ mailbridge [<recipient(s)> ...] [-f <fromaddr>] [-i] [-oi] 
	[-p [<host>:]<proto>] [-mh <mailhost>] [-ms <mailsvc>] [-V]

Arguments:
<recipient(s)>		recipients
-f <fromaddr>		address of sender
-p [<host>:]<proto>	host-protocol specification
-i			ignored
-oi			ignored
-mh <mailhost>		mailhost
-ms <mailsvc>		mail-service to use
-V			print program version to standard-error and then exit

