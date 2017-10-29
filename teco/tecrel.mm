'\"	@(#) tecrel.mm:  6.1 12/23/83
.PM BP
.TL
UNIX* TECO Release Notes
.AU "David M. Kristol" DMK MH 45415 4725 3A536
.AS
TECO is a popular editor on vendor-supplied operating systems
for PDP-8, PDP-11, VAX-11, and PDP-10 computers.
This document describes an implementation which is compatible
with TECO-11 for the PDP-11 running RSX-11 and the VAX-11 running
VAX/VMS\(dg.
.FS \(dg
PDP, RSX, VAX, and VMS are trademarks of Digital Equipment Corporation.
.FE
It runs on UNIX versions 4.2 and later on PDP-11, VAX-11,
and 3B20S processors.
.AE
.MT 1
.H 1 INTRODUCTION
.P
.FS *
UNIX is a trademark of AT&T Bell Laboratories.
.FE
TECO (\fIT\fRext \fIE\fRditor and \fICO\fRrrector)
is a popular editor on Digital
Equipment Corporation (DEC) computers such as the PDP-8, PDP-11,
VAX-11, and PDP-10.
Its source of popularity lies in its power as a text processing
language, as well as its utility as a simple editor.
Users frequently write TECO programs (called \fImacros\fR) to
do text processing instead of writing specialized programs,
much as UNIX users write shell scripts for similar purposes.
.P
The present implementation, wholly written in C, attempts to be
faithful to the letter and spirit of the PDP-11 implementation
of TECO that runs on the most popular DEC PDP-11 operating system,
RSX-11.
A similar version runs in compatibility mode under VAX/VMS.
This new implementation makes life easier for users of UNIX,
RSX-11, and VAX/VMS by providing a single common compatible editor.
.P
The standard reference for TECO is the
.IR "PDP-11 TECO User's Guide" ,
DEC Order Number DE-11-UTECA-B-D.
This document describes the differences between UNIX TECO and
the standard TECO described therein.
As a general rule, UNIX TECO provides all of the standard
facilities and all features labeled
.IR "[TECO-11 only]" .
Features labeled
.I "[TECO-10 only]"
are not implemented.
These release notes are keyed by chapter numbers to the reference
manual.
.P
Readers of this document should be familiar with UNIX and,
to some extent, with the
.I "PDP-11 TECO User's Guide."
Otherwise the comments about specific TECO commands will seem
like utter gibberish.
.H 1 "TERMINAL HANDLING"
.P
While it runs, UNIX TECO alters your terminal control modes that you may
have set with \fIstty\fR(1).
UNIX TECO resets them when it exits.
Normally you will be unaffected by these changes, especially
since TECO continues to use your \fIerase\fR, \fIline kill\fR,
and \fIinterrupt\fR characters.
However, you may be affected if you run TECO as part of a pipeline
(see below).
.P
When you write output to the terminal with the ^A command,
you must write both a Carriage Return \fBand\fR Line Feed
character, as with standard TECO.
UNIX TECO disables the special treatment of New Line for your terminal.
Carriage Return and Line Feed are treated as distinct characters.
Most of the time, however, you need not realize this.
For example, when you type Carriage Return at the
end of a line as you enter text,
TECO automatically supplies a Line Feed for you.
.P
UNIX TECO avoids changing terminal modes when it is used as
part of a pipeline, or when it runs as a background process.
These topics will be discussed below.
.P
TECO is normally configured to use a virtual terminal package
like
.I curses
or
.I termcap
to support its ``CRT rubout'' capability.
With this capability, TECO actually erases characters from a
terminal when you type your
.I erase
or
.I "line kill"
character.
This is similar to the UNIX system's
.I echoe
capability.
.H 1 "SPECIFIC NOTES"
.H 2 "Invocation (Chapter 2)"
.P
TECO supports the standard methods of invocation, except
that \fImake\fR has been supplanted by \fIcreate\fR to avoid the
obvious conflict with the UNIX \fImake\fR command.
To summarize, TECO supports these methods of invocation:
.sp
.DS
.nf
	\fBteco\fR
	\fBteco\fR filename
	\fBteco\fR newfile=oldfile	(no spaces around =)
	\fBcreate\fR newfile
	\fBmung\fR cmdfile [ args ... ]
	\fBteco\fR @cmdfile [ args ... ]
.fi
.DE
.sp
Also, to be more in tune with the UNIX system, the various options, like
/INSPECT and /NOMEMORY, have been changed to \fB-inspect\fR and
\fB-nomemory\fR.  Analogous changes apply to the other RSX-11 switches.
All of the options may be abbreviated to a unique prefix and they may
be entered in either case or mixed cases (\fIi.e.\fR, \fB-NOM\fR, \fB-nom\fR,
or \fB-Nom\fR).
.H 3 "User Initialization"
.P
UNIX TECO supports the TECO.TEC feature.
In UNIX TECO, if there is a \fIteco.tec\fR file
in your current working directory,
it is invoked as described in the RSX-11 Appendix of the
.I "PDP-11 TECO User's Guide" .
(To be more accurate, \fIteco.tec\fR may be in any directory
specified by variable \fBTECPATH\fR, which need not include
your current directory.
See the description of the EI command, below.)
UNIX TECO also permits you to have a \fIteco.ini\fR file
(instead of TECO.INI).
TECO searches for it using \fBTECPATH\fR, as well.
UNIX TECO will create a \fIteco_memory\fR file in your current
working directory (instead of the RSX-11 TECF00.TEC) to
remember the file you are editing.
Use \fB-nomemory\fR to suppress the memory function and thus avoid
creating or referring to the file.
.P
One important difference between UNIX TECO and TECO-11
is the way arguments are handled.
UNIX systems can pass many arguments, and
the arguments may contain embedded spaces.
UNIX TECO appends each of the command line arguments
(including \fIarg0\fR) to the filename buffer, followed by
a special separator character which the shell can never pass.
(There is no separator character after the last argument.)
TECO puts the numeric value of the separator character into
the numeric portion of Q-register Z, from where it may be
used in searches with the ^EUZ construct.
.H 2 "Conventions and Structures (Chapter 3)"
.H 3 "Number Size"
.P
Numbers in UNIX TECO are the C language \fIint\fR size for the host
machine.
That is, on PDP-11's, numbers are 16 bits.
They are 32 bits on the VAX-11 and 3B20.
.H 3 "Text Arguments"
.P
Text arguments in UNIX TECO are fully generalized, as they
are for TECO-11.
That means that any command taking a text argument
.RI ( e.g. ,
I\fItext\fR$)
can also be specified with the @ modifier and user-specified
delimiters
.RI ( i.e. ,
@I/\fItext\fR/).
.H 2 "Command String Editing (Chapter 4)"
.H 3 "Immediate Action Commands"
.P
The immediate action commands <DELETE> and <CTRL/U> behave differently
in UNIX TECO:  They depend on the
.I erase
and
.I "line kill"
characters you have selected with \fIstty\fR(1).
Your \fIerase\fR character supplants the <DELETE> character.
It does not echo as itself, but rather performs the erase function
as described in the manual.
Similarly, your \fIline kill\fR character serves in place of
<CTRL/U>.
TECO does echo the \fIline kill\fR character, however.
.P
UNIX TECO does not yet implement the <CTRL/Z> <CTRL/Z> <CTRL/Z>
immediate action command.
.H 3 "Operating System Filters"
.P
<CTRL/S> and <CTRL/Q> (XOFF and XON) are silently discarded by
UNIX systems.
To enter these as TECO commands, use the two-character form:
^S and ^Q.
Other characters reach UNIX TECO intact.
.H 2 "Command Summary (Chapter 5)"
.H 3 "File Specification Commands"
.H 4 "File specifiers"
.P
UNIX TECO expects file specifiers to be UNIX paths.
The path may be absolute (fully specified from /) or relative
(to the current working directory).
TECO does not assume any default suffix (or ``extension'', as the
DEC operating systems call it), such as ``.c''.
Take note that, because TECO does not use the same meta-characters
as the shell, it is possible (and rather easy) to create files
with names that are difficult to remove.
TECO makes no restrictions on what characters may appear in a
file specifier beyond what the operating system itself will
reject as a bad path name, but it will warn you when you
create a file whose name contains unusual characters.
.P
Path names are restricted to a maximum of 100 characters in
the current implementation.
.P
The EB command creates a backup file by renaming the
input file when the output file is actually closed.
The backup filename is formed by appending ``.B'' to the
original filename.
UNIX TECO generates an error message if the path for a
file is too long (too many characters),
or if it cannot create a backup file because
the resulting path name or filename is too long.
.H 4 Options
.P
None of the file specification commands have any
options (switches) at present.
.H 4 "Protection Modes"
.P
The file protection modes for new files are set as follows.
Files created with the EW command get
the current default protection mode.
Files created with EB get a copy of
the old file's protection modes.
If a file is read-only, you will not be able to use the EB command:
Access will be denied to you.
If you try to do an EW on a file which exists, one of two
things will happen.
If you do not have write permission for the file, you will be
denied access to it.
Otherwise you will receive a warning that you are superseding
an existing file.
.P
UNIX TECO does not actually overwrite an existing file when you
supersede it.
Therefore it is possible to execute the sequence
.sp
	ER\fIfile\fR$ EW\fIfile\fR$ EK$
.sp
and retain the original \fIfile\fR.
.H 4 "EI Command"
.P
The UNIX TECO EI command does not append an extension (like .tec) to
the \fIfile\fR argument.
It takes the name exactly as you provide it.
.P
UNIX TECO does have a useful extension to EI, however.
If there is a \fBTECPATH\fR variable in the environment,
UNIX TECO uses it to search for the file specified by EI.
The format of \fBTECPATH\fR is similar to that used by
shell variables \fBPATH\fR and \fBCDPATH\fR:  Paths
are separated by `:'.
You must specify a null path in \fBTECPATH\fR if you want
to search your current working directory:
.sp
	TECPATH=":$HOME:/usr/tecbin"
.sp
The path search mechanism is bypassed if \fIfile\fR begins with
`./', `../', or `/'.
If \fBTECPATH\fR is absent from the environment or is null,
\fBTECPATH\fR is treated as if it were ``:''.
.H 4 "EN Command"
.P
EN\fIfilespec\fR$ sets the wildcard lookup file specifier.
In UNIX TECO, this file specifier may be a series of file
specifiers of the sort you would pass to the shell, separated
by spaces:
.sp
	ENx.c *.h z.s$
.sp
UNIX TECO will remember all files that match the wildcard
file specifier \fBat the time the EN command is executed\fR.
Only files that actually exist at that time and that match
the file specifier will be remembered.
Use EN$ to retrieve them one at a time.
Directories can be found with EN, and the filename will be
returned by EN$ with a trailing /.
.H 4 "EG Command"
.P
The EG command in UNIX TECO behaves somewhat non-standardly.
It never exits TECO.
If an output file is open, the EG command does an effective EC command first.
Then it passes the \fItext\fR of EG\fItext\fR$ to the shell.
You get a new TECO prompt when the shell exits.
If you use the colon modifier, (:EG), the command has the value of
the exit status returned by the shell.
Beware that the success value returned by the shell differs from the
one that TECO uses, so :EG\fIcmd\fR$"S ...  may not do what you expect.
You can do a plain EG$ or :EG$ command, in which case UNIX TECO
will tell you what it is doing (``[\fIlast EG command\fR]'') and do it.
.P
In UNIX TECO the \fItext\fR portion of the EG command can be
several lines long.
You should be aware that TECO converts Carriage Return/Line Feed pairs in the
\fItext\fR into New Line characters when it passes the command(s)
to the shell.
.H 3 "Page Manipulation Commands"
.P
UNIX TECO reads standard UNIX text files.
Such files usually do not contain Carriage Return (CR) characters,
but rather contain New Line (NL) characters to denote the end of a line.
(Note that NL is the ASCII character Line Feed (LF).)
UNIX TECO lines end with CR and LF (as they do in standard TECO).
When UNIX TECO reads New Line characters in UNIX text files,
it converts them into CR/LF pairs.
Similarly, when it writes UNIX text files, UNIX TECO
converts CR/LF pairs to NL.
Isolated CR and LF characters are written to a text file unchanged.
.P
UNIX TECO retains NUL characters (ASCII 0) when reading
and writing text files.
This behavior mimics TECO-11 and is different from standard TECO.
.P
UNIX TECO behaves like standard TECO with regard to editor ``pages''.
A page normally ends with a Form Feed (FF) character.
However, end-of-file will terminate a page, as well.
Since UNIX TECO buffers text files in memory, it is possible
to fill its text buffer when you edit a very large file.
UNIX TECO attempts to leave you a reasonable amount of space to
do useful work, even when this condition arises.
Thus, it will terminate a page before it reads a FF if it comes to
the end of a line (sees a NL) and there is insufficient space
for another line plus a 500 byte cushion.
The ^E flag tells you whether a page was read up to a FF.
If TECO reads an incomplete page, the remainder may still be read
(when space is available) with P or Y commands.
.H 3 "Type-out Time Commands"
.P
<CTRL/O> behaves the same way for UNIX TECO as it does in
standard TECO:  It toggles terminal output off and on.
Even when output is turned off, TECO continues to run and
generate output, but the output gets discarded.
Contrast this with <CTRL/S> and <CTRL/Q>, which are also
implemented, and which stop and restart terminal output
without discarding any characters.
.P
Your \fIinterrupt\fR key, as set with \fIstty\fR(1), will cause
UNIX TECO to stop execution and return to prompt level.
You will get an error message, as well.
\fIInterrupt\fR will break through, even if you have inhibited
output with <CTRL/S> or <CTRL/O>.
.H 3 "Search Commands"
.P
UNIX TECO does implement the m,nS and ::S commands.
.H 3 "Q-registers"
.P
UNIX TECO permits you to specify two special Q-register names,
* (file specifier) and _ (search string) to these commands:
G\fIq\fR, :G\fIq\fR, ^EQ\fIq\fR (search string building construct).
* and _ are not allowed elsewhere as Q register names.
.H 3 "Special Numeric Values"
.P
UNIX TECO has only a few anomalies in this category.
The ^F command (switch register value) always returns 0.
^B and ^H (date and time) use the RSX-11 and VAX/VMS format.
See the
.I "PDP-11 TECO User's Guide."
On PDP-11 systems beware of the high-order bit, since these numbers
may be negative, giving rise to interesting calculations.
.P
The \fIn\fR^Q command is implemented.
It returns the number of characters between the buffer pointer and
the \fIn\fRth line separator (both positive and negative).
.sp
	\fIn\fR^QC is equivalent to \fIn\fRL
.sp
.H 3 "Environment Characteristics"
.P
These environment characteristics are defined for the EJ command
in a UNIX system:
.VL 10
.LI -1EJ
For UNIX systems, operating system = 10.
For 3B20S, computer = 10.
For PDP-11 and VAX, computer = 0 and 4 respectively.
.LI 0EJ
returns process ID.
.LI 1EJ
returns the number \fInn\fR if your terminal is named /dev/tty\fInn\fR.
Otherwise it returns -1.
.LI 2EJ
returns your user ID number.
.LE
.sp
If the argument number is less than -1, it is treated as -1.
Similarly, arguments greater than 2 are treated like 2.
.H 3 "Mode Control Flags"
.P
These peculiarities of UNIX TECO are noted:
.VL 6
.LI EH
``War and Peace'' mode is not supported.
.LI EO
UNIX TECO always returns a version number of 36.
.LI ET
.VL 10
.LI &2
(Scope mode) When set, CRT rubout mode is enabled.
When clear, TECO does its normal erase and line kill processing.
.LI &64
(Detach flag) When set, means the \fIinterrupt\fR key is ignored.
This bit is set when UNIX TECO runs as a background process.
If the bit is cleared, TECO responds to \fIinterrupt\fR, even if
it is running as a background process.
.LI &256
(Truncate lines)
When set, output lines are truncated to the width of your terminal.
If your terminal type is unrecognized, the width is set to 72.
If this bit is clear, long lines wrap around on your terminal.
.LI &512
(Scope ``WATCH'') Not supported yet.
Always zero.
.LI &1024
(Refresh scope ``WATCH'') Not supported.
Always zero.
.LE
.LE
.H 3 "Scope Commands"
.P
UNIX TECO does not yet support the W command or the
VTEDIT feature of TECO.
.H 2 "Extensions"
UNIX TECO has extensions to standard TECO for UNIX systems only.
You should of course beware that using UNIX-only
commands gives rise to non-portable TECO macros.
The details of I/O redirection and pipes that are given below
may be ignored at a first reading, as they are intended
for more unusual uses of UNIX TECO.
.H 3 "Exit Codes"
TECO can return an exit code to the shell.
The EX and ^C commands take an optional numeric argument which
is taken as the exit code to return to the shell.
The default value is zero.
In addition, TECO uses these error codes:
.VL 5
.LI 0
when TECO exits because you typed ^C ^C at command level.
.LI 1
for an error during TECO's start-up processing.
.LI 2
if an error occurs during a \fBmung\fR.
.LE
.H 3 "EE Command"
The EE command lets you access UNIX environment variables \fIvia\fR a
two-step process.
The first step specifies the variable name and places its value into
a special Q-register.
The second step is to access that Q-register.
The special Q-register has the name $ and may be used in the same
contexts as special Q-registers * and _.
Q$ is read-only.
.P
You load Q$ by executing an EE\fIvarname\fR$ command.
TECO searches your environment for \fBvarname\fR and places the
variable's value in Q$.
If the search fails, Q$ is left empty, and TECO prints an ``Environment
variable not found'' error message.
The : modifier may be used to suppress the error message and return
a success/failure value as to whether the variable exists.
The @ modifier may be used to select alternate delimiters for
the EE command.
.H 3 "CRT Rubout Mode"
CRT rubout mode refers to TECO's ability to erase characters
from your terminal when you type your
.I erase
or
.I line kill
characters.
Non-UNIX TECOs usually just support this mode on DEC VT-52 and
VT-100 terminals.
UNIX TECO supports CRT rubout mode on any terminal that it
recognizes and that has what it considers to be the necessary
terminal capabilities.
.P
TECO must be built with one of these video terminal handling
packages to provide CRT rubout capabilities:
.IR curses ", " termlib ", or " termcap .
When TECO starts executing, it looks for the
.B TERM
environment variable and determines whether that terminal can
support CRT rubout mode,
and, if it does, TECO sets the ET&2 bit automatically to enable
the mode.
Otherwise the ET&2 bit is cleared, and attempts
to set it by TECO commands will be ignored.
.H 3 "I/O Redirection"
.P
When UNIX TECO starts running, it checks whether standard input
is coming from a terminal.
To facilitate the use of pipes and I/O redirection, standard input
is left open if the input is not a terminal.
(Commands are obtained from other than standard input.)
A similar test is done for standard output.
Thus you could run UNIX TECO by saying
.sp
	\f(CWteco -nomemory <infile >outfile\fR
.sp
Since TECO never ``sees'' the I/O redirection, you must supply
the \fB-nomemory\fR option to suppress TECO's memory.
Otherwise TECO may attempt to edit the last file it edited, and
it will discover that a file (standard output) is already open.
.H 3 "TECO and Pipes"
.P
UNIX TECO can detect when it is being used as part of a pipeline.
In such cases it retains the normal Carriage Return and Line Feed
behavior according to your \fIstty\fR modes.
Thus if your terminal is the ultimate output sink for the pipeline,
lines will look normal.
(Otherwise, TECO changes the modes so CR and LF are distinct
characters.
Since New Line is the same as LF, and since TECO converts CR/LF
pairs to NL on output, the output on your terminal would look like
a staircase.)
A consequence of not changing CR/LF behavior is that,
if you use TECO interactively in
the pipeline (which is permitted), you will get double CRs at
the end of each line.
TECO will also behave non-standardly when you back up over a Line Feed
and/or a Carriage Return.
.P
When you use \fIteco\fR (as opposed to \fImung\fR) as part of
a pipeline, you should probably use the \fB-nomemory\fR
option to suppress the memory function.
Otherwise TECO may get confused when it tries to open a file
it had been editing previously.
.P
When you run TECO as part of a foreground pipeline, beware that
typing \fIinterrupt\fR will likely kill the other foreground
processes.
(TECO will continue to run.)
If TECO is not the last process in the pipeline, the shell will
think the pipeline has exited, and it will prompt you.
Thereafter, TECO and the shell will compete for input characters,
leading to great confusion.
To make safe use of TECO interactively in a pipeline, use a
shell script like this (named \f(CWcatch\fR) to catch \fIinterrupt\fR:
.sp
.nf
	\f(CWtrap "" 2
	exec sh -c "$1"\fR
.sp
.fi
.ad
and use it like this:
.sp
.nf
	\f(CWcatch " ... | teco -nom | ... "\fR
.sp
.fi
.ad
When TECO starts up, the ET&64 bit will be set because TECO
will think it is running as a background process and
it will disable \fIinterrupt\fR.
(See next section.)
You should do a 64,0ET command to clear the bit, enabling the
\fIinterrupt\fR function.
Otherwise you will be unable to stop infinite loops except
by disconnecting your terminal line.
.H 3 "TECO as a Background Process"
.P
UNIX TECO may be run as a background process.
This usually makes sense only when TECO is invoked via \fImung\fR.
Otherwise TECO will compete with the shell for input characters,
no doubt to your great befuddlement.
UNIX TECO assumes it is running as a background process if it
discovers that the \fIinterrupt\fR signal is being
ignored.
When TECO decides that it is running as a background process,
it avoids changing terminal modes as long as possible.
That is, it waits until it must request input from the terminal.
.H 3 "TECO and Signals"
.P
UNIX TECO handles the UNIX signals described below in a specific way.
Others are handled in the default way.
In particular, there is currently no special handling for
the hang-up signal.
.VL 10
.LI SIGINT
is used to interrupt TECO command execution.
If \fIstty\fR mode \fIbrkint\fR is set, the BREAK key behaves
like \fIinterrupt\fR.
.LI SIGQUIT
is used to provide standard TECO's ^O feature (kill output).
.LI SIGPIPE
is ignored.
An output error (?OUT) will ultimately result when output is
done to the output stream that is connected to the pipe.
.LI SIGSOFT
behaves like SIGINT.
A UNIX \fIkill\fR command directed at a background TECO process
will force TECO to abort, just as if the \fIinterrupt\fR key had been
used.
.LE
.H 1 "INSTALLING TECO"
.P
TECO is distributed as a collection of these files in one directory:
.BL
.LI
C source files.
.LI
Makefile.
.LI
Documentation source files.
.LE
.H 2 "Building TECO"
.P
Just running the
.I make
command in TECO's directory is usually enough to build TECO successfully.
Problems that arise are usually related to the availability of
a virtual terminal package (\fIe.g.\fR,
.IR curses ,
.IR termcap ,
or
.IR termlib ).
On PDP-11s, the
.I curses
package is too big to fit with TECO (as of this writing).
Follow the procedure described below for when there is no
virtual terminal package.
.H 2 "In Case of Difficulty"
.P
Some common problems, and how to solve them, are outlined below.
.H 3 "Loader can't find \fIlibcurses.a\fR"
The makefile assumes that the virtual terminal package on your
system is
.IR curses .
If, instead, you have
.I termcap
or
.I termlib
available, try
.sp
.ce
\f(CWmake CURSES=-ltermcap\fR (or \f(CWCURSES=-ltermlib\fR)
.sp
to select the package you have available.
.H 3 "None of the virtual terminal packages are available"
.P
Modify 
.I mdconfig.h
by
.IR #undef ining
CRTRUB.
Then do a
.sp
.ce
\f(CWmake CURSES=\fR
.sp
.H 3 "Symbols \f(CWospeed\fP and \f(CWPC\fP are multiply defined"
Your C compiler does not support multiply defined external definitions.
Edit
.I crt.c
and place ``extern'' in front of the declarations for
\f(CWospeed\fR and \f(CWPC\fR.
.H 2 "Installing TECO"
.P
Choose a convenient directory in which to place the
.I teco
executable program and copy it there.
Make links to it with the names
.I create
and
.I mung
(\f(CWln teco create; ln teco mung\fR).
Place the manual page in an appropriate place for local manual pages.
.sp
.SG dmk
