CONFERENCE

This is the PCS Conference client program.  The PCS conference server
is PCSCS (PCS Conference Server).

Synopsis:
$ conference [-u username] [-j {channel|+|-}] [-s server]

Only the actual system user can assume the username that corresponds to
a system username.  The default conference username is the same as the
invoking system username.

User commands:
User name			login with given name
Join [channel]			join a channel (for listening)
LEave [channel]			leave a channel
Direct [-u] {channel | user}	direct input to a channel
Private user [text]		direct input to a user only
Ls [-a]				list channels listening on
Info username			provide information about username
Info [-c] channel		provide information about channel
LInes n				number of dialogue lines at bottom (default 2)
LOg [-c channel] name		create a named log of session or channel

