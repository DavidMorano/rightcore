.\" TECO Reference Card	@(#) refcard.mm:	5.1 12/6/83
.\" This file is designed to produce a pocket reference card.
.\" For best results, use troff and send the result to a photo-typesetter.
.\" The output comes out in 3 x 9 in. pages which must be cut and pasted
.\" to produce 7 (gasp) 8.5 x 11 in. pages, with each big page containing
.\" 3 of the little pages.
.\" The proper command to use to get output is:
.\"	mmt -t -rW3i -rL9i [this file]
.tr ~
.de VI
.LI "\\$1"
.if \w"\\$1"+1m>\\n(:bu .br
..
.SA 1
.S 8 10
.br
.ne 10
.HU General~Information
Throughout this card, the following notation will be used:
.VL 8
.VI \fB\s+2$\s-2\fR
represents an ESCAPE (ASCII code 033).
On older terminals, there is a key marked ALTMODE which is
treated the same way.
.VI \fB\s+2^X\s-2\fR
represents the single control character CONTROL/X.
.VI ^X
represents either \fB\s+2^X\s-2\fR (one character) or the
two-character sequence CARET (or UP-ARROW), followed by X.
.VI \fIn\fR
represents a numeric value specified by a digit string or which
is the result of a TECO command that returns a value.
.VI \fItext\fR
represents an arbitrary string of text.
Most commands taking a \fItext\fR argument have, as their
standard terminator, \fB\s+2$\s-2\fR.
However, all such commands have an alternate form which
is denoted by a preceding @, the command, and an arbitrary
separator which must not appear in \fItext\fR.
For example, I\fItext\fR\fB\s+2$\s-2\fR and @I/\fItext\fR/
are equivalent commands.
.VI \fIq\fR
represents a Q-register name, which may be an upper or lower case letter
or a digit.
In some commands \fIq\fR may be one of these characters:
* _ $.
.LE
.P
Although commands are shown here as upper case characters, the
equivalent lower case character may be used as well.
.P
The @ forms of commands taking \fItext\fR arguments are not
shown explicitly here.
.sp 2
.ne 10
.HU Invoking~TECO
.P
You can invoke TECO with these commands:
.sp
.DS
.TS
;
l | l .
Command	Similar to...
_
\f(CWteco\fR \fIoldfile\fR	EB\fIoldfile\fR\fB\s+2$\s-2\fRY
\f(CWteco\fR	\f(CWteco\fR \fIremembered-file\fR
\f(CWteco\fR \fInewfile=oldfile\fR	ER\fRoldfile\fR\fB\s+2$\s-2\fREW\fI\
newfile\fR\fB\s+2$\s-2\fRY
\f(CWcreate\fR \fInewfile\fR	EW\fInewfile\fR\fB\s+2$\s-2\fR
\f(CWmung\fR \fIcmdfile\fR [ \fIargs ... \fR]	I\fIargs ...\fR\fB\s+2$\s-2\fR\
EI\fIcmdfile\fR\fB\s+2$\s-2\fR\fR\fB\s+2$\s-2\fR 
\f(CWteco\fR \fI@cmdfile\fR [ \fIargs ... \fR]	\f(CWmung\fR \fIcmdfile\fR \
[ \fIargs ... \fR]
.TE
.DE
.sp 2
.P
These options also apply.
The optional part appears as \fIitalics\fR.
.VL 15
.VI -noc\fIreate\fR
Do not automatically create a new file.
.VI -noi\fIni\fR
Suppress use of \fIteco.ini\fR.
.VI -nom\fIemory\fR
Suppress the memory function.
Don't use remembered file or remember this one.
.VI -in\fIspect\fR
Inspect.
Open for reading only.
.LE
.sp 2
.ne 10
.HU Immediate~Mode~Commands
.VL 10
.VI \fB\s+2$\s-2\fR\fB\s+2$\s-2\fR
Execute command string.
.VI <\fIerase\fR>
UNIX erase character deletes last character in command string.
.VI <\fIkill\fR>
UNIX line kill character deletes current line of command string.
.VI \fB\s+2^C\s-2\fR
Delete current command string.
.VI "\fB\s+2^C\s-2\fR \fB\s+2^C\s-2\fR"
Exit TECO immediately.
.VI "\fB\s+2^G\s-2\fR \fB\s+2^G\s-2\fR"
Delete current command string.
.VI "\fB\s+2^G\s-2\fR <space>"
Type current line of command string.
.VI "\fB\s+2^G\s-2\fR *"
Type entire command string.
.VI LF
Line Feed (or \fB\s+2^J\s-2\fR) moves pointer forward
one line and prints it.
.VI BS
Backspace (or \fB\s+2^H\s-2\fR) moves pointer backward
one line and prints it.
.VI ?
After error, type command string up to command that caused error.
.VI *\fIq\fR
Store previous command string in Q-register \fIq\fR.
.LE
.sp 2
.ne 10
.HU Execution~Time~Commands
.VL 10
.VI <\fIinterrupt\fR>
Stop command execution.
.VI \fB\s+2^O\s-2\fR
Turn off type out; toggles.
.VI \fB\s+2^S\s-2\fR
(XOFF)  Halt terminal output.
(Depends on \fIstty\fR modes \fBixon\fR and \fBixany\fR.)
.LE
.sp 2
.ne 10
.HU File~Specification~Commands
.VL 15
.VI ER\fIfilespec\fR\fB\s+2$\s-2\fR
Opens \fIfilespec\fR for input.
.VI EW\fIfilespec\fR\fB\s+2$\s-2\fR
Opens \fIfilespec\fR for output.
.VI EB\fIfilespec\fR\fB\s+2$\s-2\fR
Opens \fIfilespec\fR for both input and output, and with backup protection.
.VI :ER\fIfilespec\fR\fB\s+2$\s-2\fR
Same as ER\fIfilespec\fR\fB\s+2$\s-2\fR, except returns value:
-1 if file found, 0 if file not found.
.VI :EW\fIfilespec\fR\fB\s+2$\s-2\fR
Same as EW\fIfilespec\fR\fB\s+2$\s-2\fR, except returns value:
-1 if file does not yet exist, 0 if file already exists.
.VI :EB\fIfilespec\fR\fB\s+2$\s-2\fR
Same as EB\fIfilespec\fR\fB\s+2$\s-2\fR, except returns value:
-1 if file found, 0 if file not found.
.VI EN\fIfilespec\fR\fB\s+2$\s-2\fR
Sets up \fIfilespec\fR as a wildcard file specification.
.VI EN\fB\s+2$\s-2\fR
Gets the next wildcard match into the file specifier buffer
(Q-register *).
.VI :EN\fB\s+2$\s-2\fR
Same as EN\fB\s+2$\s-2\fR, but returns value:
-1 if another file found, 0 if no more files.
.LE
.sp 2
.ne 10
.HU File~Termination~Commands~and~Exit
.VL 15
.VI EF
Closes the output file.
.VI EC
Writes remainder of buffer to output file, copies remaining input
to the output file, closes the output file.
.VI EX
Same as EC, then exits TECO.
.VI EK
Forgets the current output file.
.VI EG\fIcommand\fR\fB\s+2$\s-2\fR
\fINon-standard\fR.
If text buffer contains text, do EC.
In either case, pass \fIcommand\fR to the shell.
.VI EG\fB\s+2$\s-2\fR
Same as EG\fIcommand\fR, but execute the \fIcommand\fR
last specified by EG\fIcommand\fR\fB\s+2$\s-2\fR.
.VI :EG\fIcommand\fR\fB\s+2$\s-2\fR
.VI :EG\fB\s+2$\s-2\fR
Same as corresponding EG command, but return the executed command's
exit code as the value of the command.
.LE
.sp 2
.ne 10
.HU Auxiliary~File~Commands
.VL 15
.VI EP
Switch to secondary input stream.
.VI ER\fB\s+2$\s-2\fR
Switch to primary input stream.
.VI EA
Switch to secondary output stream.
.VI EW\fB\s+2$\s-2\fR
Switch to primary output stream.
.VI EI\fIfilespec\fR\fB\s+2$\s-2\fR
Open \fIfilespec\fR for command input.
Takes effect at end of current command string.
.VI EI\fB\s+2$\s-2\fR
Switch command input back to terminal, disregard further
input from a command file.
Takes effect at end of current command string.
.VI :EI\fIfilespec\fR\fB\s+2$\s-2\fR
Same as EI\fIfilespec\fR\fB\s+2$\s-2\fR, except returns value
(immediately):  -1 if file found, 0 if not found.
.LE
.sp 2
.ne 10
.HU Page~Manipulation~Commands
.VL 8
.VI A
Appends next page of input file to text buffer
.VI :\fIn\fRA
Appends \fIn\fR lines of text to text buffer and returns value:
-1 if success, 0 if no lines read.
.VI Y
``Yank'' buffer load.
Equivalent to HKA.
.VI EY
``Yank'' buffer load and bypass ``yank protection''.
(See ED~&~2.)
.VI P
Output the entire text buffer, clear the buffer, and read the
next page of the input file.
Append a form feed if the last read terminated with a form feed.
.LE
.P
A, Y, EY, and P have a : form (\fIe.g.\fR, :Y) which returns
a value:  -1 if success, 0 if no lines read.
.VL 8
.VI \fIn\fRP
Do \fIn\fR P commands.
.VI \fIm,n\fRP
Output characters in positions \fIm\fR through \fIn\fR (inclusive)
in the text buffer.
.VI PW
Output current text buffer and append a form feed.
.VI \fIn\fRPW
Output current page \fIn\fR times, appending a form feed each time.
.VI \fIm,n\fRPW
Output characters in positions \fIm\fR through \fIn\fR (inclusive)
in the text buffer (without a form feed).
.LE
.sp 2
.ne 10
.HU Buffer~Pointer~Positioning~Commands
.VL 8
.VI \fIn\fRJ
Move pointer to immediately following the \fIn\fRth character in
the text buffer.
.VI 0J
Move pointer to beginning of text buffer.
.VI J
Same as 0J.
.VI \fIn\fRC
Move buffer pointer forward (to higher numbered character positions)
\fIn\fR characters.
.VI C
Same as 1C.
.VI -C
Same as -1C.
.VI \fIn\fRR
Move buffer pointer backward \fIn\fR characters.
(Same as -\fIn\fRC.)
.VI R
Same as 1R.
.VI -R
Same as -1R.
.VI \fIn\fRL
Move pointer to beginning of \fIn\fRth line from current
buffer position.
.VI 0L
Move to beginning of current line.
.VI L
Move to beginning of next line.
Same as 1L.
.VI -L
Move to beginning of previous line.
Same as -1L.
.LE
.sp 2
.ne 10
.HU Buffer~Positions
.VL 8
.VI B
First buffer position, equal to zero.
.VI Z
Last buffer position, equal to the number of characters in
the buffer.
.VI \fB.\fR
Current pointer position, equal to the number of characters
in the buffer preceding the pointer.
.VI \fIm\fR,\fIn\fR
(\fIm\fR+1)st through \fIn\fRth characters in the text buffer.
.VI H
The entire buffer.
Same as B,Z.
.VI ^Y
Same as .+^S,.~.
This gives the bounds for the last insert or search match.
.LE
.sp 2
.ne 10
.HU Deletion~Commands
.VL 8
.VI \fIn\fRD
Delete \fIn\fR characters following the current pointer.
.VI -\fIn\fRD
Delete \fIn\fR characters preceding the current pointer.
.VI D
Same as 1D.
.VI -D
Same as -1D.
.VI \fIm\fR,\fIn\fRD
Delete characters in the text buffer between positions \fIm\fR and
\fIn\fR.
The pointer moves to position \fIm\fR.
.VI \fIn\fRK
Delete all characters in the text buffer from the current buffer
pointer position to the beginning of the \fIn\fRth line from
there (\fIi.e.\fR, the point reached by an \fIn\fRL command).
.VI K
Same as 1K.
.VI -K
Same as -1K.
.VI 0K
Delete all characters from the beginning of the current line to the
current buffer pointer.
.VI \fIm\fR,\fIn\fRK
Same as \fIm\fR,\fIn\fRD.
.LE
.sp 2
.ne 10
.HU Insertion~Commands
.P
For all of these commands, text is inserted at the current
buffer pointer, and the pointer is advanced to just beyond
the current pointer.
.VL 10
.VI I\fItext\fR\fB\s+2$\s-2\fR
Insert \fItext\fR.
.VI \fIn\fRI\fB\s+2$\s-2\fR
Insert the character whose ASCII code is \fIn\fR.
.VI <tab>\fItext\fR\fB\s+2$\s-2\fR
Same as I\fItext\fR\fB\s+2$\s-2\fR, except the first character
inserted is a <tab>.
.VI \fIn\fR\e
Insert a character numeric representation of \fIn\fR.
The representation depends on the current radix.
.VI FR\fItext\fR\fB\s+2$\s-2\fR
Replace last string inserted or found in a search by \fItext\fR.
Same as ^SDI\fItext\fR\fB\s+2$\s-2\fR.
.LE
.sp 2
.ne 10
.HU Arithmetic~Operators
.P
Operators are evaluated from left to right.
Arithmetic precision is the natural integer size on
the host machine:  16 bits on PDP-11's, 32 bits on 3B20 and VAX.
.VL 8
.VI +\fIn\fR
Same as \fIn\fR
.VI -\fIn\fR
Negative of \fIn\fR
.VI \fIm\fR+\fIn\fR
Addition
.VI \fIm\fR-\fIn\fR
Subtraction
.VI \fIm\fR*\fIn\fR
Multiplication
.VI \fIm\fR/\fIn\fR
Integer division
.VI \fIm\fR&\fIn\fR
Bitwise logical AND
.VI \fIm\fR#\fIn\fR
Bitwise logical inclusive OR
.VI \fIn\fR^_
One's complement of \fIn\fR
.VI (\fIn\fR)
Performs enclosed operations first
.LE
.sp 2
.ne 10
.HU Radix~Control
.P
Current radix affects interpretation of literal numbers
and the \e and \fIn\fR\e commands.
.VL 8
.VI ^O
Set octal radix.
.VI ^D
Set decimal radix.
.VI \fIn\fR^R
Sets radix to \fIn\fR, which must be 8, 10, or 16.
.VI ^R
Returns current radix.
.LE
.sp 2
.ne 10
.HU Type-Out~Commands
.P
Type-out commands produce their output on your terminal.
.VL 8
.VI \fIn\fRT
Type all text from the current pointer position to the beginning of
the \fIn\fRth line from there (the point reached by an \fIn\fRL command).
.VI \fIm\fR,\fIn\fRT
Type characters between buffer positions \fIm\fR and \fIn\fR.
.VI T
Same as 1T.
.VI -T
Same as -1T.
.VI 0T
Types the current line up to the current pointer position.
.VI HT
Types the entire buffer.
.VI \fIn\fRV
Same as (1-\fIn\fR)T\fIn\fRT.
.VI \fIm\fR,\fIn\fRV
Same as (1-\fIm\fR)T\fIn\fRT.
.VI V
Type current line.
Same as 0TT.
.VI ^A\fItext\fR\fB\s+2^A\s-2\fR
Type \fItext\fR.
.VI \fIn\fR=
Type value of \fIn\fR as a signed decimal number, followed by CR/LF.
.VI \fIn\fR==
Type value of \fIn\fR as an octal number, followed by CR/LF.
.VI \fIn\fR===
Type value of \fIn\fR as a hexadecimal number, followed by CR/LF.
.LE
.P
For each of the three commands above, the CR/LF at the end may be
suppressed by a prefixed `:', as in \fIn\fR:=~.
.sp
.VL 8
.VI \fIn\fR^T
Type the character whose ASCII code is \fIn\fR.
.VI ^L
Type a form feed.
.LE
.sp 2
.ne 10
.HU Search~Commands
.P
The ^X flag affects case recognition in the search commands.
Bits in the ED flag affect these aspects of searches:
meaning of ^, whether failing searches affect `.'~,
how much to move `.' on multiple occurrence searches,
``yank protection''.
The ES flag controls automatic type-out on search match.
Successful searches always leave the buffer pointer after the
text that was found.
Failing searches normally move the pointer to position 0.
.P
All search commands may be modified by a preceding `:',
which causes the command to return a value.
The returned value is 0 if the search fails, and -1 if the search succeeds.
.ne 10
.VL 10
.VI \fIn\fRS\fItext\fR\fB\s+2$\s-2\fR
Search for the \fIn\fRth occurrence of \fItext\fR between
`.' and the end of the text buffer.
.VI S\fItext\fR\fB\s+2$\s-2\fR
Same as 1S\fItext\fR\fB\s+2$\s-2\fR.
.VI -\fIn\fRS\fItext\fR\fB\s+2$\s-2\fR
Same as \fIn\fRS\fItext\fR\fB\s+2$\s-2\fR, except the search
proceeds backward from `.' to position 0.
.VI \fIn\fRN\fItext\fR\fB\s+2$\s-2\fR
Same as \fIn\fRS\fItext\fR\fB\s+2$\s-2\fR, except that if
too few occurrences of \fItext\fR are found on the current
page, successive pages are input and output (with the equivalent
of a P command) until \fIn\fR occurrences are found or the
entire input file has been passed to the output file.
.VI N\fItext\fR\fB\s+2$\s-2\fR
Same as 1N\fItext\fR\fB\s+2$\s-2\fR.
.VI \fIn\fR_\fItext\fR\fB\s+2$\s-2\fR
Similar to \fIn\fRN\fItext\fR\fB\s+2$\s-2\fR, except that only
input is done, not output.
.VI _\fItext\fR\fB\s+2$\s-2\fR
Same as 1_\fItext\fR\fB\s+2$\s-2\fR.
.VI \fIn\fRE_\fItext\fR\fB\s+2$\s-2\fR
Same as \fIn\fR_\fItext\fR\fB\s+2$\s-2\fR, except that ``Yank protection''
is bypassed.
(See ED~&~2.)
.VI E_\fItext\fR\fB\s+2$\s-2\fR
Same as 1E_\fItext\fR\fB\s+2$\s-2\fR.
.VI \fIn\fRFS\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR
Replace the \fIn\fRth occurrence of \fItext1\fR in the buffer
by \fItext2\fR.
The \fIn\fRth occurrence is located by an \fIn\fRS\fItext1\fR\fB\s+2$\s-2\fR.
.VI FS\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR
Same as 1FS\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR.
.VI \fIn\fRFN\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR
Similar to \fIn\fRFS\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR,
but use an N search instead of an S search.
That is, buffer boundaries may be crossed.
.VI FN\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR
Same as 1FN\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR.
.VI \fIn\fRF_\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR
Similar to \fIn\fRFS\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR,
but use an _ search instead of an S search.
That is, buffer boundaries may be crossed.
.VI F_\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR
Same as 1F_\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR.
.LE
.sp 2
.ne 10
.HU Bounded~Searches
.P
These searches share all of the attributes of the preceding
searches, except that `.' does not change if the search fails.
.VL 10
.VI \fIm\fR,\fIn\fRFB\fItext\fR\fB\s+2$\s-2\fR
Search for \fItext\fR between buffer positions \fIm\fR and \fIn\fR.
The matching \fItext\fR must begin within those bounds but can
end outside.
The search is a forward search if m~<~n and backward if m~>~n.
.VI \fIn\fRFB\fItext\fR\fB\s+2$\s-2\fR
Same as \fIr\fR,\fIs\fRFB\fItext\fR\fB\s+2$\s-2\fR, where
\fIr\fR~=~. and \fIs\fR is the position reached by an \fIn\fRL command.
.VI FB\fItext\fR\fB\s+2$\s-2\fR
Same as 1FB\fItext\fR\fB\s+2$\s-2\fR.
.VI -FB\fItext\fR\fB\s+2$\s-2\fR
Same as -1FB\fItext\fR\fB\s+2$\s-2\fR.
.LE
.P
FC commands are a search and replace version of FB
(\fIe.g.\fR,
\fIm\fR,\fIn\fRFC\fItext1\fR\fB\s+2$\s-2\fR\fItext2\fR\fB\s+2$\s-2\fR).
.VL 10
.VI ::S\fItext\fR\fB\s+2$\s-2\fR
Compare command.
If the characters following the current pointer match \fItext\fR,
::S moves the pointer beyond \fItext\fR and returns -1.
Otherwise it returns 0.
Same as .,.:FB\fItext\fR\fB\s+2$\s-2\fR.
.VI \fIm\fR,\fIn\fRS\fItext\fR\fB\s+2$\s-2\fR
Same as \fIn\fRS\fItext\fR\fB\s+2$\s-2\fR, but the search
does not cross more than \(orm\(or~-~1 characters.
.VI 0,\fIn\fRS\fItext\fR\fB\s+2$\s-2\fR
Same as \fIn\fRS\fItext\fR\fB\s+2$\s-2\fR, except the
search position remains unchanged on search failure.
.VI \fIm\fR,S\fItext\fR\fB\s+2$\s-2\fR
Same as \fIm\fR,1S\fItext\fR\fB\s+2$\s-2\fR.
.VI \fIm\fR,-S\fItext\fR\fB\s+2$\s-2\fR
Same as \fIm\fR,-1S\fItext\fR\fB\s+2$\s-2\fR.
.LE
.sp 2
.ne 10
.HU Match~Control~Constructs
.P
These constructs are valid only within a search string
(\fIi.e.\fR, \fItext\fR above).
.VL 8
.VI \fB\s+2^X\s-2\fR
Matches any character.
.VI \fB\s+2^S\s-2\fR
Matches any non-alphanumeric separator.
.VI \fB\s+2^N\s-2\fR
Matches any character \fIexcept\fR a character matched by the
following character (or match construct).
.VI \fB\s+2^E\s-2\fRA
Matches any alphabetic character (upper and lower case).
.VI \fB\s+2^E\s-2\fRB
Same as \fB\s+2^S\s-2\fR.
.VI \fB\s+2^E\s-2\fRC
Matches any symbol constituent:  A-Z, a-z, 0-9, _.
.VI \fB\s+2^E\s-2\fRD
Matches any digit.
.VI \fB\s+2^E\s-2\fRG\fIq\fR
Matches any character in Q-register \fIq\fR.
.VI \fB\s+2^E\s-2\fRL
Matches any line terminator:  LF, FF, VT.
.VI \fB\s+2^E\s-2\fRR
Matches any alphanumeric character.
.VI \fB\s+2^E\s-2\fRS
Matches any non-null string of spaces and TABs.
.VI \fB\s+2^E\s-2\fRV
Matches any lower case alphabetic character.
.VI \fB\s+2^E\s-2\fRW
Matches any upper case alphabetic character.
.VI \fB\s+2^E\s-2\fRX
Same as \fB\s+2^X\s-2\fR.
.LE
.sp 2
.ne 10
.HU Search~String~Building~Characters
.P
These constructs are used to build up a search string.
They may also be used to build \fIfilespec\fR in the
file manipulation commands.
.VL 8
.VI \fB\s+2^Q\s-2\fR
Use the next character literally.
.VI \fB\s+2^R\s-2\fR
Same as \fB\s+2^Q\s-2\fR.
.VI \fB\s+2^V\s-2\fR
Converts the next character, if alphabetic, to lower case.
.VI \fB\s+2^W\s-2\fR
Converts the next character, if alphabetic, to upper case.
.VI ^
If ED~&~1 is zero, convert the following character to the
equivalent control character.
.VI \fB\s+2^E\s-2\fRQ\fIq\fR
Use the string stored in Q-register \fIq\fR at this point.
\fIq\fR may be one of the special Q-registers *, _, or $.
.VI \fB\s+2^E\s-2\fRU\fIq\fR
Use the character whose ASCII code is in the numeric portion
of Q-register \fIq\fR here.
.LE
.sp 2
.ne 10
.HU Numeric~Quantities
.VL 8
.VI \fIn\fRA
Numeric value corresponding to the ASCII code for
the character at position .~+~\fIn\fR in the buffer.
0A gives the value for the character immediately following the pointer.
.VI \e
Has the value of the numeric string immediately following the
buffer pointer, as interpreted in the current radix.
The buffer pointer moves just past the end of the string.
.VI ^B
Is an encoding of the current date:
((year-1900)*16+month)*32+day~.
.VI ^E
Form feed flag.
-1 if current buffer has an implicit FF following it, 0 otherwise.
.VI ^F
Always zero.
(Value of console switch register.)
.VI ^H
Is an encoding of the time of day:
(seconds since midnight)/2.
On 16-bit systems, this number can be negative.
Beware!
.VI ^N
End of file flag.
-1 if current input file is at end of file, 0 otherwise.
.VI \fIn\fR^Q
Number of characters between the buffer pointer and the \fIn\fRth line
separator from there.
\fIn\fR^QC is equivalent to \fIn\fRL.
.VI ^S
Negative of length of last insert, string found by a search,
or string inserted with G command, whichever occurred last.
.VI ^T
Numeric value of ASCII code for next character typed at the
terminal.
(See ET~&~32.)
.VI ^Z
Total space occupied by text in Q-registers and other internal
text storage areas (apart from the text buffer).
.VI ^^\fIx\fR
Numeric value of ASCII code for character \fIx\fR.
.LE
.sp 2
.ne 10
.HU Conditional~Commands
.P
The general forms of a conditional command are
.sp
.ce
\fIn\fR"\fIXthen-commands\fR'~~~(if-then)
.sp
and
.sp
.ce
\fIn\fR"\fIXthen-commands\fR~|~\fIelse-commands\fR'~~~(if-then-else).
.sp
\fIn\fR is a number or expression (which can include
the results of commands that return values).
\fIX\fR is one of the letters given below.
\fIThen-commands\fR are executed if the \fIX\fR condition
is true, and \fIelse-commands\fR are executed otherwise.
Conditionals may be nested.
.P
In this list, the ``true'' condition about \fIn\fR is stated.
.VL 8
.VI A
\fIn\fR equals the ASCII code for an alphabetic character.
.VI C
\fIn\fR equals the ASCII code for a (C language) symbol constituent:
letters, digits, and _.
.VI D
\fIn\fR equals the ASCII code for a digit.
.VI E
\fIn\fR = 0.
.VI F
\fIn\fR represents False (=~0).
.VI G
\fIn\fR > 0.
.VI L
\fIn\fR < 0.
.VI N
\fIn\fR \(!= 0.
.VI R
\fIn\fR equals the ASCII code for an alphanumeric character.
.VI S
\fIn\fR represents Success (<~0).
.VI T
\fIn\fR represents True (<~0).
.VI U
\fIn\fR represents Unsuccessful (=~0).
.VI V
\fIn\fR equals the ASCII code for a lower case alphabetic character.
.VI W
\fIn\fR equals the ASCII code for a upper case alphabetic character.
.VI <
\fIn\fR < 0.
.VI >
\fIn\fR > 0.
.VI =
\fIn\fR = 0.
.LE
.sp 2
.ne 10
.HU Iteration~and~Flow~Control
.VL 10
.VI \fIn\fR<\fIcommands\fR>
Performs the enclosed \fIcommands\fR \fIn\fR times.
If \fIn\fR~\(<=~0, the \fIcommands\fR are skipped.
.VI <\fIcommands\fR>
Performs \fIcommands\fR an ``infinite'' number of times.
.VI !\fItag\fR!
Define \fItag\fR as a program label.
(May also be considered as a comment.)
.VI O\fItag\fR\fB\s+2$\s-2\fR
Continue command execution after label \fItag\fR.
\fItag\fR must be in the same macro as the O command.
.VI \fIn\fRO\fItag0\fR,\fItag1\fR,\fItag2\fR,...\fB\s+2$\s-2\fR
Continue execution at the place marked by \fItag0\fR,
\fItag1\fR, \fItag2\fR, \fIetc.\fR, if \fIn\fR has the value
0, 1, 2, \fIetc.\fR
If \fIn\fR is out of range, continue execution following the O
command.
.VI ;
Leave the current iteration (<...>) if the immediately preceding
search command failed.
.VI \fIn\fR;
Leave the current iteration if \fIn\fR~\(>=~0.
.VI :;
Leave the current iteration if the immediately preceding search
command succeeded.
.VI \fIn\fR:;
Leave the current iteration if \fIn\fR~<~0.
.VI F>
Branch to end of current iteration (and test count).
Outside of an iteration, exit the current macro.
.VI F<
Branch to beginning of current iteration (or macro), don't test count.
.VI F'
Branch to the end of the current conditional.
.VI F|
Branch to the \fIelse\fR clause of the current conditional.
.VI \fB\s+2$\s-2\fR\fB\s+2$\s-2\fR
Exit from the current macro level.
.VI \fIn\fR\fB\s+2$\s-2\fR\fB\s+2$\s-2\fR
Exit and return the value \fIn\fR from the current macro.
.VI \fIm\fR,\fIn\fR\fB\s+2$\s-2\fR\fB\s+2$\s-2\fR
Exit and return values \fIm\fR and \fIn\fR from the current macro.
.VI ^C
From a macro, return to TECO's command level.
From TECO's command level, exit TECO.
.VI ^C\fB\s+2^C\s-2\fR
Abort the current command string and return to UNIX.
.LE
.sp 2
.ne 10
.HU Q-Register~Loading
.P
The U and % commands affect the numeric portion of Q-registers.
The X and ^U commands affect the text portion of Q-registers.
The ] command affects both.
.VL 10
.VI \fIn\fRU\fIq\fR
Store integer \fIn\fR in Q-register \fIq\fR.
.VI \fIm\fR,\fIn\fRU\fIq\fR
Store \fIn\fR in Q-register \fIq\fR and return value \fIm\fR.
.VI \fIn\fR%\fIq\fR
Increment Q-register \fIq\fR by \fIn\fR and return result.
.VI %\fIq\fR
Same as 1%\fIq\fR.
.VI \fIn\fRX\fIq\fR
Copy characters in buffer positions from `.' to the \fIn\fRth
line separator from there into Q-register \fIq\fR.
.VI \fIm\fR,\fIn\fRX\fIq\fR
Copy characters in buffer positions \fIm\fR through \fIn\fR
into Q-register \fIq\fR.
.VI X\fIq\fR
Same as 1X\fIq\fR.
.VI -X\fIq\fR
Same as -1X\fIq\fR.
.VI \fIn\fR:X\fIq\fR
Same as \fIn\fRX\fIq\fR, except the text is appended to
Q-register \fIq\fR, instead of replacing the old contents.
The same applies to:  \fIm\fR,\fIn\fR:X\fIq\fR,
:X\fIq\fR, -:X\fIq\fR.
.VI ^U\fIqtext\fR\fB\s+2$\s-2\fR
Store \fItext\fR in Q-register \fIq\fR.
.VI :^U\fIqtext\fR\fB\s+2$\s-2\fR
Append \fItext\fR to Q-register \fIq\fR.
.VI \fIn\fR^U\fIq\fB\s+2$\s-2\fR
Store the character whose ASCII code is \fIn\fR in Q-register \fIq\fR.
.VI \fIn\fR:^U\fIq\fB\s+2$\s-2\fR
Append the character whose ASCII code is \fIn\fR to Q-register \fIq\fR.
.VI ]\fIq\fR
Pop from the Q-register push-down stack into Q-register \fIq\fR.
Both text and number portions are affected.
.VI :]\fIq\fR
Same as ]\fIq\fR, but returns a value:
-1 if successful, 0 if the stack was empty.
.LE
.sp 2
.ne 10
.HU Q-Register~Retrieval
.VL 8
.VI Q\fIq\fR
Returns the numeric value in the numeric portion of Q-register \fIq\fR.
.VI \fIn\fRQ\fIq\fR
Returns the ASCII code of the \fIn\fRth character in the text
portion of Q-register \fIq\fR, or -1.
.VI :Q\fIq\fR
Returns number of characters stored in the text portion of Q-register \fIq\fR.
.VI G\fIq\fR
Inserts the text portion of Q-register \fIq\fR into the text buffer
at the current pointer position.
The pointer moves to just beyond the inserted text.
\fIq\fR may be one of the special Q-registers:
.VL 5
.LI *
file specifier buffer
.LI _
search string buffer
.LI $
environment variable string.
(See EE command.)
.LE
.VI :G\fIq\fR
Types the contents of Q-register \fIq\fR on your terminal.
.VI M\fIq\fR
Executes the text in Q-register \fIq\fR as TECO commands (a TECO ``macro'').
\fIn\fRM\fIq\fR and \fIm\fR,\fIn\fRM\fIq\fR do the same thing, except
\fIn\fR, or \fIm\fR and \fIn\fR, is passed to the macro as numeric
arguments.
.VI [\fIq\fR
Pushes the numeric and text portions of Q-register \fIq\fR onto
the Q-register push-down stack.
.LE
.sp 2
.ne 10
.HU Mode~Control~Flags
.P
These flags control various aspects of TECO's behavior.
They may be read or set in a consistent way with the
commands shown below.
<\fIflag\fR> stands for any of the flags in the table below.
.VL 13
.VI <\fIflag\fR> 
Returns the value of <\fIflag\fR>
.VI \fIn\fR<\fIflag\fR>
Sets the value of <\fIflag\fR> to \fIn\fR.
.VI \fIm\fR,\fIn\fR<\fIflag\fR>
Turns off bits specified by \fIm\fR and turns on bits specified by \fIn\fR
in <\fIflag\fR>.
.VI 0,\fIn\fR<\fIflag\fR>
Turns on bits specified by \fIn\fR.in <\fIflag\fR>.
.VI \fIm\fR,0<\fIflag\fR>
Turns off bits specified by \fIn\fR in <\fIflag\fR>.
.LE
.sp
.P
These are the <\fIflag\fR>'s.
.VL 5
.LI ED
Edit mode flag.
Initial value is 1.
.ne 8
.TS
;
c c l
c l l
c c l.
Bit	Value	Explanation
Position		
1	0	T{
^ in search string makes following character a
control character
T}
~ 	1	^ in search string is itself
_
2	0	T{
Y and _ fail if an output file is open and text exists
in the text buffer (``yank protection'')
T}
~	1	Allow all Y and _ commands
_
4	0	Expand memory as much as needed
~	1	Only expand memory for A command
_
8	0	Always
_
16	0	T{
Search failure moves pointer to 0
T}
~	1	T{
Search failure leaves pointer unchanged
T}
_
32	0	Always
_
64	0	T{
Skip entire matching string on multiple search
T}
~	1	T{
Move pointer by 1 after each match in multiple search
T}
.TE
.sp
.VI EH
Help level flag.
Initial value is 0.
.VL 8
.LI EH~&~3
If 0, 2, or 3, error messages contain ?XXX error code and short message.
If 1, error messages consist of ?XXX only.
.LI EH~&~4
0:  no error trace-back.
1:  Simulate ? after error (provide trace).
.LE
.VI EO
TECO version flag.
Value is 36 for UNIX TECO.
.VI ES
Search verification flag, initially 0.
Upon a search match at TECO's command level,
ES causes the following behavior, according to its value:
.VL 8
.VI 0
No type-out.
.VI -1
Equivalent to V.
.VI 1~-~31
Equivalent to 0T 10^T T:  type current line with line feed
just after the cursor.
.VI "32~-~126"
Same as previous case, but type character whose code is ES instead
of LF.
.VI "127~-~255"
Same as previous case, but type null character instead of LF.
.VI >~256
Treat ES value as \fIn\fR~*~256~+~\fIc\fR, where
\fIn\fR is the number of lines to view (as in \fIn\fRV), and \fIc\fR 
is the ASCII code of the character to display at the cursor position.
.LE
.VI ET
Terminal mode flag.
These bits may be set by start-up code:
ET~&~4, ET~&~64, ET~&~128, ET~&~512.
Others are zero.
.ne 8
.TS
;
c c l
c l l
c c l.
Bit	Value	Explanation
Position		
1	0	T{
Control characters are typed in ^X form
T}
~	1	T{
Characters are typed in image mode, with no conversion
T}
_
2	0	T{
Character and line erase behave standardly
T}
~	1	T{
Character and line erase actually erase characters (``scope mode'')
T}
_
4	0	T{
Convert lower case input to upper case
T}
~	1	Read lower case input literally
_
8	0	Echo characters read by ^T
~	1	T{
Suppress echo of characters read by ^T
T}
_
16	0	(Always reads as 0)
~	1	T{
Set by command string to cancel ^O on type out
T}
_
32	0	^T waits for next character
~	1	T{
^T returns character if one is waiting, or -1 otherwise
T}
_
64	0	Respond to interrupt character
~	1	Ignore interrupt
_
128	0	Normal error action
~	1	Exit TECO on error
_
256	0	Normal line output
~	1	T{
Truncate typed lines at line length
T}
_
512	0	Terminal is not CRT
~	1	Terminal is a recognized CRT
_
1024	0	Always
_
32768	0	Interrupt behaves normally
~	1	T{
Interrupt clears this bit, but processing continues
T}
.TE
.sp
.VI EU
Upper/lower case flag.
Initial value is -1 if your terminal can handle lower
case output; otherwise, 0.
Behavior depends on flag value:
.VL 5
.LI 0
Lower case characters are flagged with '
.LI <~0
No case flagging
.LI >~0
Upper case characters are flagged with '
.LE
.VI EV
Edit verify flag, initially 0.
EV values work the same way as for ES, but the type-out occurs
just before TECO's prompt.
.VI ^X
Search mode flag.
Initially 0.
If zero, searches are alphabetic case-insensitive.
.tr ~~
Also characters ` { | } and ~ match @ [ \e ] and ^ respectively.
.tr ~ 
If non-zero, searches must match identically.
.LE
.sp 2
.ne 10
.HU Environment~Commands
.VL 10
.VI \fIn\fREJ
Returns various environment characteristics, depending on the value
of \fIn\fR.
Numbers less than -1 are treated as -1, and numbers greater than 2
are treated as 2.
.VL 4
.LI -1
Returns a number of the form \fIm\fR~*~256~+~\fIn\fR that
describes the computer system on which TECO is running.
\fIn\fR is always 10 on UNIX systems.
\fIm\fR specifies the processor:
.VL 4
.LI 0
PDP-11
.LI 4
VAX-11
.LI 10
3B20S
.LE
.LI 0
Returns TECO's process ID.
.LI 1
Returns the number \fInn\fR if your terminal is named /dev/tty\fInn\fR.
Otherwise it returns -1.
.LI 2
Returns your user ID number.
.LE
.VI EE\fIname\fR\fB\s+2$\s-2\fR
\fINon-standard\fR.
Considers \fIname\fR as the name of a UNIX environment variable
and searches for it.
The value of the variable is placed in Q-register $.
.VI :EE\fIname\fR\fB\s+2$\s-2\fR
Same as EE\fIname\fR\fB\s+2$\s-2\fR, except returns a value:
-1 if the name was found, 0 if not.
.LE
.sp 2
.ne 10
.HU Miscellaneous~Commands
.VL 4
.VI ?
Toggles command tracing.
.VI \fB\s+2$\s-2\fR
Discards values.
.LE
.de TX
.S 16 18
.ce
\fBTECO Reference Card\fR
.S 8 10
.sp .25i
This card provides a quick reference to UNIX* TECO.
It does not give elaborate information.
For complete details on any command, consult the
.I "PDP-11 TECO User's Guide".
.sp .75i
.\" Put footnote in by hand at bottom of page
.BS
-------
.br
* UNIX is a trademark of Bell Laboratories.
.BE
..
.TC
