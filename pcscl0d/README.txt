PCSCL

This is the PCS "Content-Length" adder program. This program basically copies
input files (specified on as invocation arguments) to a mailbox (or standard
output as the mailbox). The mailbox is file-record-locked in accordance with the
"mailbox" locking requirement. Note that the input files CANNOT be mailbox files
as no locking is done on them.

This program adds "content-length" headers for mail messages for delivery to a
mailbox. A few other headers are also added under certain circumstances.

This program differs from programs like 'mail.local' and '/bin/mail -d' in that:

1) this program works correctly in many cases whereas they fail under
   several circumstances

2) this program does not deliver to username mailboxes located
   in a fixed local file but rather delivers to mailbox files specified
   on invocation or to standard output (as a mailbox)

3) this program will properly handle System V UNIX type mail locking
   and will delete old mail locks when encountered

4) this program can accept multiple input files as specified
   on invocation (else it reads standard input)

The program configuration file can contain the following keys :

key		default program value
-------------------------------------------------

helpfile	lib/pcscl/help (or other standard place and name)
logfile		log/pcscl
tmpdir		/tmp
loglen		80*1024

