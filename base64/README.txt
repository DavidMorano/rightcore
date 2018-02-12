BASE64

This program can be used to either BASE64-encode or BASE64-decode a file.
Although BASE64 has been adopted for email message bodies, this program has
nothing to do with email messages.  If you have an email message body (for
example) that needs decoding, it is the user's responsibility to remove all of
the email message framework stuff and only feed the actual BASE64 encoded part
to this program.

Synopsis:
$ base64 {-e|-d} [<file>] [-t] [-<cols>] [-V]

Arguments:
<file>		file to process
-e		encode mode
-d		decode mode
-t		specify text encoding
-<cols>		?
-V		prince program version to standard-error and then exit

