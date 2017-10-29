static char SCCSID[] = "@(#) tty.c:  6.1 8/25/83";
/* tty.c
 *
 *	TECO terminal handling
 *
 *	David Kristol, March, 1982
 *
 *	This module contains the system-dependent terminal control
 *	code.  The idea is that calls are made to routines in this
 *	module to set up the terminal for proper behavior during
 *	several phases of TECO processing, to wit:
 *	 -- during command reading
 *	 -- during command execution with wait for ^T
 *	 -- during command execution with no wait for ^T
 *
 *	These ideas are encapsulated by
 *		Ttscmd
 *		Ttsxec
 *		Ttsxecnd (no delay)
 *
 *	Terminal initialization is also handled here.  We create two
 *	standard i/o (stdio) streams, one for input from the terminal
 *	(reading commands and ^T) and writing character echoes.  We
 *	also set up the stream "Cmdout" which is normally the terminal
 *	but can be redirected (via file descriptor 3) to a file.
 *	Furthermore, we initialize the case control flags:  EU and
 *	ET_rlc.
 *
 *	Other routines:
 *
 *		Ttreset		resets the terminal to its entry mode
 *		Ttsave		saves the current terminal mode
 *		Ttrestore	restores it.
 *		Ttynum		fetches terminal # for user's terminal
 *		Ttspeed		returns terminal line speed as encoded by
 *				stty
 */



/* Terminal and signal handling is quite messy.  We'll say a few
** words here to try to clarify things.  The messiness comes from the
** ability to use TECO via "mung" in a pipeline, for example, and in
** the background.
*
** Terminal handling
**
**	Because TECO can be used in ways other than interactively
** with direct communications with the keyboard, special precautions
** must be made.  In general, on UNIX systems, TECO must alter the
** terminal modes to deal with CR and LF separately.  However, it's
** unwise to change terminal modes indescriminately, since the user
** may have said "mung foo&", and we wouldn't want the foreground
** terminal modes diddled.  Therefore we take the precaution not to
** modify the stty modes until the first time they get set up for
** command input when TECO is used in a situation other than normal
** interactive editing.  This semi-hack assures that nothing funny will
** happen to the terminal for "mung's" in the background or in pipelines.
**
** Signals
**
**	TECO uses the interrupt and quit signals, normally, to assist
** its processing.  Interrupt is used to abort command string execution.
** quit is subverted to provide the ^O (kill output) function of DEC
** TECO.  The broken pipe signal is ignored, since the routine that's
** writing to it will give a write error.  The software termination
** signal is treated as "interrupt", and is used to kill TECO when it
** runs in the background.
**
**	TECO tests the interrupt signal status on entry.  If it is set
** to "ignore", TECO assumes that it is running in the background.
** TECO then sets the ET & 64 bit to say that the terminal is detached,
** in the sense that interrupt should be ignored.  Setting ET & 64 to
** 0 will make TECO interruptable again.
*/



#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termio.h>
#include	<string.h>
#include	<stdlib.h>
#include <stdio.h>

#include "bool.h"
#include "chars.h"
#include "ctype.h"
#include "errors.h"
#include "errmsg.h"
#include "tflags.h"
#include "values.h"			/* to define TNUMB */



/* various global and local data */

#define	STDIN		0	/* standard input file descriptor */
#define	STDOUT		1	/* standard output file descriptor */
#define CMDOUTFD	3	/* file descriptor to check for command
				 * output
				 */

/* declare offsets in c_cc array of terminal mode */

#define QUIT	1		/* quit character */
#define ERASE	2		/* char erase char */
#define KILL	3		/* line kill char */
#define MIN	4		/* min. chars (same as EOF slot) */
#define TIME	5		/* min. time (same as EOL slot) */



/* external subroutines */

extern int	atoi() ;

extern char	*ttyname();

extern void	interr() ;			/* declare function */


/* external variables */

extern TNUMB ET;		/* TECO ET flag */
extern TNUMB EU;		/* TECO EU flag */


/* local global data */

short Charkill;			/* single character kill character */
short Linekill;			/* line kill character */

BOOL Xnlcr;			/* TRUE if incoming NL's on the terminal
				** must be interpreted as CR's
				*/
int Twidth;			/* terminal width (in chars) */
static int Tnum;		/* user's terminal number */

static short Tty;		/* terminal file descriptor */

FILE * Ttyin;			/* terminal input stream */
FILE * Ttyout;			/* terminal output stream */
FILE * Cmdout;			/* command output stream:  ^A, T, V, W, etc. */

/* terminal status for various conditions */

static struct termio entrymode;	/* terminal mode on startup */
static struct termio cmdmode;	/* terminal mode for command reading */
static struct termio xecmode;	/* terminal mode for command execution */
static struct termio xecndmode;	/* terminal mode for command execution when
				 * ^T wait is turned off (No Delay)
				 */
static struct termio savedmode;	/* mode saved by Ttsave */

static struct termio * curmode;	/* pointer to most recent terminal mode */

static BOOL interrupt;		/* interrupt signal */
BOOL Fl_ctO;			/* ^O flag */
static BOOL ok_to_change;	/* TRUE if it's okay to change terminal
				** stty modes
				*/


/* some basic routines */


/* ttset -- set terminal mode from 'termio' structure
 *
 * This routine sets the terminal control modes from a previously
 * prepared structure.
 */

static void
ttset(buf)
struct termio * buf;		/* pointer to 'termio' buffer */
{
    int ioctl();

    if (ok_to_change && (curmode != buf)) /* check for new mode */
    {
	curmode = buf;		/* save this as current mode */
	if (ioctl(Tty,TCSETAW,buf) < 0)
	    interr("ttset failed");
    }
    return;
}
/* Ttinit -- initialize terminal handling
 *
 * This routine sets up the various terminal control structures for later
 * use.  It also sets up the Cmdout stream, and gets the number of the
 * users terminal.
 */

void
Ttinit()
{
    BOOL ispipe();
    void siginit();

    int flag = fcntl(CMDOUTFD,F_GETFL,0);
				/* returned file status flags */
    BOOL inpipe = ispipe(STDIN); /* TRUE if input is a pipe */
    BOOL outpipe = ispipe(STDOUT); /* TRUE if output is a pipe */
/* (Do this here, before we create another fd with open) */

    char * tname;		/* terminal name string */
    int i;			/* loop index */
    char * s;			/* pointer to file mode string */

/* Before changing any file descriptors, try to find out what the user's
** terminal number is.  This turns out to be hard on UNIX systems.  The
** usual assumption, which we follow here, is that one of units 0 - 2
** is the user's terminal.  If we try to find out what the terminal
** name is for "/dev/tty", we get that string back again.  For the others
** we have a chance of seeing something like "/dev/tty03".
**
** For our terminal I/O we will open /dev/tty03 (for example) if possible.
** As a last resort we will use /dev/tty.  This will keep up-to-date
** the access time that programs like "finger" use, so administrators
** won't think the user is sleeping at the terminal.
*/

	Tnum = -1;			/* set assumed terminal number
					** if all else fails
					*/
    
    for (i = 2, tname = NULL; i >= 0 && tname == NULL; i--)
	tname = ttyname(i);		/* try to get terminal name string */

    if (strncmp(tname,"/dev/tty",8) == 0 && ISDIG(*(tname+8)))
	Tnum = atoi(tname + 8);		/* if right prefix, get terminal # */

    if (tname == NULL)
	tname = "/dev/tty";		/* our "last resort" name */
/* open Tty file */

    if ((Tty = open(tname,O_RDONLY)) < 0)
	interr("can't open Tty");

/* now open terminal input and output streams */

    if ((Ttyin = fdopen(Tty,"r")) == NULL)
	interr("can't fdopen tty in");

    if ((Ttyout = fdopen(open(tname,O_WRONLY),"w")) == NULL)
	interr("can't fdopen tty out");

/* Now set up Cmdout.  If file descriptor 3 is not set suitably, just use
 * Ttyout by default.
 */

    Cmdout = Ttyout;		/* assume default case */
    s = (char *) 0;		/* set default */

    if (flag == O_WRONLY)	/* normal open for write */
	s = "w";		/* set mode string */
    else if (flag == O_RDWR)
	s = "r+";		/* mode is read/write */
    if (s != (char *) 0)	/* if not null string */
	if ((Cmdout = fdopen(CMDOUTFD,s)) == NULL)
	    Cmdout = Ttyout;	/* reset to default if failed */
/* now set up various terminal control mode settings */

/* first, save current setting */

    if (ioctl(Tty,TCGETA,&entrymode) < 0)
	interr("can't save terminal mode in Ttinit");

/* get current editing characters */

    Charkill = entrymode.c_cc[ERASE]; /* character erase */
    Linekill = entrymode.c_cc[KILL];  /* line kill */

/* Initialize case flags according to this table:
**
**	IUCLC	OLCUC		EU	ET_rlc
**	 bit	 bit		flag	 bit
**
**	  0	  0		-1	  1	reads, writes lc
**	  0	  1		+1	  1	reads lc, flag uc output
**	  1	  0		-1	  0	no read lc, write both
**	  1	  1		0	  0	upper case only
*/

/* assume ET, EU currently zero */

    if ( (entrymode.c_iflag & IUCLC) == 0)
	ET |= ET_rlc;
    
    if ( (entrymode.c_oflag & OLCUC) == 0)
	EU = -1;			/* handle funny case next */
    
    if ( EU == 0 && (ET & ET_rlc) != 0 )
	EU = 1;

/* Set terminal width */

    Twidth = 72;			/* assume this width for now */
/* copy initial mode, set up for command reading */
/* Leave the mode relating to New Line alone if we're in a pipeline. */

    cmdmode = entrymode;
    cmdmode.c_iflag |= ISTRIP | IGNBRK;	/* strip input to 7 bits, ignore
					 * break
					 */
    if (! (inpipe || outpipe))
	cmdmode.c_iflag &= ~(INLCR | IGNCR | ICRNL);
				/* leave CR and NL (LF) alone */
    cmdmode.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE | ECHOK);
				/* turn off interrupts, editing, echoing */
    if (! (inpipe || outpipe))
	cmdmode.c_oflag &= ~(ONLCR | OCRNL | ONLRET);
				/* turn off special CR and NL (LF) handling on
				 * output
				 */
    cmdmode.c_cc[MIN] = 1;	/* minimum of 1 character */
    cmdmode.c_cc[TIME] = 0;	/* infinite timeout */

    /* we will have to convert incoming NL's to CR's if the terminal
    ** driver is converting CR to NL
    */

    Xnlcr = (cmdmode.c_iflag & ICRNL) != 0;

/* now set up for command execution */

    xecmode = entrymode;	/* copy again */
    xecmode.c_iflag |= ISTRIP;	/* strip to 7 bits */
    if (! (inpipe || outpipe))
	xecmode.c_iflag &= ~(INLCR | IGNCR | ICRNL);
				/* leave CR and NL (LF) alone */
    xecmode.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK); /* no editing, echo */
    if (! (inpipe || outpipe))
	xecmode.c_oflag &= ~(ONLCR | OCRNL | ONLRET);
    xecmode.c_cc[MIN] = 1;	/* minimum 1 character read */
    xecmode.c_cc[TIME] = 0;	/* no timeout */
    xecmode.c_cc[QUIT] = CTRLO;	/* change quit character to ^O for
				 * output cancel
				 */

/* finally, set up for execution with no delay for character reading */

    xecndmode = xecmode;	/* same as xecmode, except... */
    xecndmode.c_cc[MIN] = 0;	/* 0 characters minimum */
    xecndmode.c_cc[TIME] = 1;	/* timeout in 0.1 seconds */

    /* it's okay to change terminal modes right away if TECO is not
    ** involved in a pipeline
    */

    ok_to_change = ! (inpipe || outpipe);

    setbuf(Ttyin,NULL);		/* set null buffers */
    setbuf(Ttyout,NULL);
    setbuf(Cmdout,NULL);

    siginit();			/* initialize signals */
    return;
}
/* siginit -- initialize signals
**
** This routine initializes handling of signals that are important
** at program initialization.
*/

static void
siginit()
{
    void catchterm();

    if (signal(SIGINT,SIG_IGN) == SIG_IGN)
    {
	ET |= ET_ttydet;	/* if ignoring interrupt, call the terminal
				** detached
				*/
	ok_to_change = FALSE;	/* don't change terminal modes until going
				** for terminal input if in background
				*/
    }
    (void) signal(SIGPIPE,SIG_IGN);	/* ignore broken pipes, take error */
    (void) signal(SIGTERM,catchterm);	/* catch software termination signal */
    return;
}
/* Ttenable -- enable changes to terminal modes
**
** This routine enables the various terminal mode setting changes.
** Otherwise, terminal mode changes are bypassed.
*/

void
Ttenable()
{
    ok_to_change = TRUE;	/* just set flag */
    return;
}
/* Ttscmd -- set terminal mode for command reading
 *
 * This routine changes the terminal mode to be appropriate for reading
 * command characters from the terminal.
 */

void
Ttscmd()
{
    (void) signal(SIGINT,SIG_IGN); /* ignore quit and interrupt */
    (void) signal(SIGQUIT,SIG_IGN);

    Fl_ctO = FALSE;		/* turn off ^O and interrupt flags */
    interrupt = FALSE;

    ttset(&cmdmode);		/* set appropriate set of modes */
    return;
}


/* ttsxec1 -- used by two routines below */

static void
ttsxec1(m)
struct termio * m;		/* set of modes to use */
{
    void catchint();
    void catchquit();

    (void) signal(SIGINT,catchint);	/* set interrupt and quit routines */
    (void) signal(SIGQUIT,catchquit);
    interrupt = FALSE;			/* no pending interrupt */
    Fl_ctO = FALSE;			/* no ^O flag */

    ttset(m);
    return;
}
/* Ttsxec -- set terminal mode for command execution
 *
 * This routine sets the terminal into the appropriate mode for command
 * execution.  The terminal is set to wait for read attempts on ^T.
 */

void
Ttsxec()
{
    ttsxec1(&xecmode);			/* set up using these modes */
    return;
}


/* Ttsxecnd -- set terminal mode for command execution, no delay
 *
 * This routine sets the terminal into the appropriate mode for command
 * execution as above.  However, the terminal is set so read attempts
 * on ^T do not delay.  They return -1 if no character is available.
 */

void
Ttsxecnd()
{
    ttsxec1(&xecndmode);		/* set up, using this set of modes */
    return;
}


/* TTreset -- reset terminal to original state */

void
Ttreset()
{
    ttset(&entrymode);			/* reset to entry state */
    return;
}
/* Ttsave -- save terminal mode status */

void
Ttsave()
{
    if (ioctl(Tty,TCGETA,&savedmode) < 0) /* attempt to save modes */
	interr("can't save terminal mode in Ttsave");
    
    return;
}


/* Ttrestore -- restore saved terminal mode status */

void
Ttrestore()
{
    ttset(&savedmode);			/* restore saved modes */
    return;
}
/* Ttynum -- return number of user's terminal
** If the terminal number is unknown, return -1.
*/

int
Ttynum()
{
    return(Tnum);			/* return number from initialization */
}
/* Ttspeed -- return terminal's stty-encoded speed */

short
Ttspeed()
{
    return(entrymode.c_cflag & CBAUD);	/* pass out the original speed */
}
/* Interrupt handling stuff */

/* test for interrupt */

void
Testinterrupt()
{
    if (! interrupt)
	return;				/* get out if not interrupt */
    
    Ttscmd();				/* reset to command modes */
    terrNUL(&Err_XAB);    		/* abort execution */
}


/* catch an interrupt signal */

static void
catchint(i)
int i;					/* signal number (ignored) */
{
    extern TNUMB ET;			/* depends on ET flag */

/* we set the interrupt flag (which stops execution eventually) if the
 * user commands haven't set the ET ^C bit
 */

    (void) signal(SIGINT,catchint);	/* re-arm interrupt */

    if ((ET & ET_ctC) != 0)
	ET &= ~ET_ctC;			/* just turn off bit */
    else if ((ET & ET_ttydet) == 0)	/* note interrupt if not detached */
	interrupt = TRUE;		/* set interrupt flag */
    return;
}

static void
catchterm(i)
int i;					/* signal number */
{
    (void) signal(i,catchterm);		/* reset trap */
    interrupt = TRUE;			/* force termination */
    return;
}


/* catch quit signal = ^O:  stop/restart output */

static void
catchquit(i)
int i;					/* signal number (ignored) */
{
    (void) signal(SIGQUIT,catchquit);	/* re-arm interrupt */

    Fl_ctO = ! Fl_ctO;			/* complement current state */
    return;
}
/* ispipe -- is file descriptor a pipe
**
** This routine makes a stab at determining whether a given file
** descriptor corresponds to a pipe or not.  We try to find out
** by doing an lseek to see whether we get an error code back.
*/

BOOL
ispipe(fd)
int fd;
{
    extern long lseek();
    extern int errno;

    /* Our strategy is to do a seek that doesn't move a file pointer,
    ** and to check for a returned error code.
    */
    return(lseek(fd,0L,1) < 0 && errno == ESPIPE);
}
