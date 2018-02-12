MSGBOX

This is the cheapy little program, called as a daemon from DWD, to post a
message to active user terminal emulation sessions and to deliver it as email to
the user's 'msgbox' mailbox.

Don't look for a lot of perfection in this program or the messy little cheapy
programs that this one calls ! This program gets the job done for now with
reasonable quality so it lives for the time being.

This program basically just calls two other programs that do the real work. One
is 'rmsg' and the other is 'deliver'.

Possible Improvements:

1) Add an envelope "from" address to the message by invoking with
   a '-f' option with the from_address as its value.  This program will
   in turn call the 'deliver' program with a '-f' option.

DONE.

I also added the following:

1) -u mailuser
   which mailuser to send email to

2) -c comment
   allow for a general comment (usually the filename of the message
   from DWD)

Future enhancements are to accept some sort of date to be used as the date of
the message when sending the email.

