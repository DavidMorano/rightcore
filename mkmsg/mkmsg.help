MKMSG

We make a mail message from input and optional attachment files.

Synopsus:
$ mkmsg [<address(s)> [...]] [-af <afile>] [-t <content_type>] [-e <encoding>] 
	[-s <subject>] [-f <fromaddr>] [-c <ccaddr>] [-b <bccaddr>]
	[-atf <attfile>] [-a <attspec>] [-o <opt(s)>]
	[-if <ifile>] [-of <ofile>] [-n] [-V]

Grammar:
content_spec := type | { type "/" subtype } | file_extension
attspec := filename | { content_spec "=" filename }
encoding := 7bit | 8bit | binary | base64

Arguments:
<address(es)>		a recipient email address for the message
-n			do not read STDIN
-if <ifile>		the primary input file
-of <ofile>		output file rather than STDOUT
-af <afile>		argument-list file
-s <subject>		message subject
-f <from_address>	message "from" address
-c <cc_address>		message "cc" address
-b <bcc_address>	message "bcc" address
-r <file>		message recipient list file
-a <attachment_spec>	a file to attach to the message
-t <content_type>	use this content-type for following attachments
-e <encoding>		use this encoding for following attachments
-atf <file>		a file with attachment specifications in it
-header <name>=<text>	additional message header
-o <opt(s)>		option(s): inline, mime
-disclaimer <file>	disclaimer text for MIME multipart messages
-d <type>=<value>	date specification
-V			print version and exit

Note that the '-t' and '-e' options do NOT affect standard input when read in
without the '-i' option. The use of the '-i' option assumes the use of '-n'!

The date specfication has the following types:
	current, now, touch, tt, ttouch, toucht, log, logz, strdig

