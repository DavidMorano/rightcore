MSGMAN (Message Manager)

Description:

This program allows programs (Shell Scripts) to manage the messages that they
send out to other facilities.

Synopsis:
$ msgman <key>[=<n>] [-n <n>] [-t <file>] {-s <msg>|-sf <msgfile>|-c}

Arguments:
<key>[=<n>]	key
-n <n>		?
-t <file>
-s <msg>	the message

Example:
SF=${HOME}/var/statmsg/filelinking.sm
$ msgman filelinking -t ${SF} -s "this is a message"

