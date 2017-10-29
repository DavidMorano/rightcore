static char SCCSID[] = "@(#) getcmd.c:  6.1 8/25/83";
/* getcmd.c
 *
 *	TECO command reader
 *
 *	David Kristol, February, 1982
 *
 *	This module contains the routines that read a command and store
 *	it in the TECO command buffer.  The only externally visible
 *	routine is 'Rdcmd', which leaves a command string in text block
 *	Q_cmd.
 */



/* These symbols are reasons for exiting rdcmd1.  Others, not listed,
 * are actual character codes.  Thus, the special ones are negative
 */

#define GOTCMD	(-1)			/* read a command */
#define DELETED (-2)			/* deleted all characters */

#include <stdio.h>			/* standard i/o */
#include "bool.h"			/* define Booleans */
#include "chars.h"			/* define character names */
#include "cmdio.h"			/* define command-related routines */
#include "cmutil.h"			/* define command utilities */
#include "errors.h"			/* define error structures */
#include "errmsg.h"			/* define error messages and routines */
#include "find.h"			/* define find routines */
#include "qname.h"			/* define Q_cmd */
#include "sources.h"			/* input sources */
#include "tb.h"				/* define text block routines */
#include "tflags.h"			/* define TECO flag bits */
#include "tty.h"			/* define terminal routines */
#include "values.h"			/* value handling */
#include "xec.h"			/* define execute routines */



/* external subroutines */

extern void	inicmd() ;		/* declare functions */
extern void	Dovflag() ;


/* external variables */

extern TNUMB	ET;			/* TECO ET flag */
extern TNUMB	EV;			/* TECO EV flag */

extern BOOL	Fl_trace;		/* trace flag */

extern int	Inpsrc;			/* input source:  values defined
					** in sources.h */


/* Rdcmd -- read a command
 *
 * This routine returns when it has a non-empty command in the
 * command buffer.  It takes care of certain "immediate" commands
 * which are only executed immediately after a prompt:
 *
 *	LF	output next text buffer line or do EV output
 *	BS	output previous text buffer line or do EV output
 *	?	do command trace
 *	*q	save previous command buffer in Q-register q
 *
 */

void
Rdcmd()
{
    void doprompt();
    BOOL doimmed();
    int rdcmd1();


    int flag = 0;			/* flag remembers command state */


    Fl_trace = FALSE;			/* turn off tracing for new
					** command string
					*/

    do
    {					/* until we get a command */
	do
	{				/* as long as we keep doing
					 * immediate commands
					 */

	    inicmd();			/* prepare for new command */
	    doprompt();			/* output prompt character */

	} while ((doimmed() ? flag=0, TRUE : FALSE));
					/* reset flag when done all
					 * immediate commands
					 */

	TBkill(Q_cmd);			/* delete all characters */
	TBreorg();			/* make maximum amount of
					 * space available for commands
					 */
    } while ((flag = rdcmd1(flag)) != GOTCMD);
    return;
}
/* doimmed -- do immediate command
 *
 * This routine handles immediate commands, those that are performed
 * immediately after a prompt.  If commands are not coming from the user's
 * terminal, we exit immediately.
 */

static BOOL				/* TRUE if we do one */
doimmed()
{
    void Dotrace();			/* declare functions */
    short Qnum();
    void ccrlf();
    void cecho();
    extern BOOL Had_error;		/* TRUE if last cmd resulted in error */

    int c;				/* next command char */
    int qnum;				/* Q-register number */
    BOOL oldHad_error = Had_error;	/* remember whether last cmd ended
					** in error
					*/

    Had_error = FALSE;			/* declare we no longer have error
					** in command string
					*/

    if (whCch() != SRC_TTY)		/* exit if next command character is
					** not from user's terminal
					*/
	return(FALSE);


    switch (c = rCch())			/* get character, dispatch */
    {
    case '*':				/* save last command in Q-reg */
	cecho(c);			/* echo original char */
	cecho(c = rCch());		/* get next char, echo */
	if ((qnum = Qnum(c,FALSE)) >= 0)
					/* check for good register (not
					 * _ or *, either
					 */
	{
	    if (TBsize(Q_cmd) == 0)	/* make sure we haven't already
					** done this
					*/
		{
		    ccrlf();		/* put message on new line */
		    terrWRN(&Wrn_SAV);	/* already saved ... */
		}
	    else
	    {
		TBrename(Q_cmd,qnum);	/* otherwise, rename Q register */
		ccrlf();		/* output cr, lf */
	    }
	}
	else				/* bad Q-register number */
	    terrCHR(&Err_IQN,c);
	return(TRUE);			/* pretend success in all cases
					 * here
					 */

    case LF:				/* line feed:  next line */
	Xecimmed("@^a/\r/l");		/* execute L */
	Dovflag( EV != 0 ? EV : -1 );	/* output current line or EV output */
	return(TRUE);			/* did immediate command */

    case BS:				/* back space:  previous line */
	Xecimmed("@^a/\r/-l");		/* execute -L */
	Dovflag( EV != 0 ? EV : -1 );	/* output current line or EV output */
	return(TRUE);			/* did an immediate command */

    case '?':				/* trace */
	if (oldHad_error)		/* only do if had error on last cmd */
	{
	    Dotrace();			/* trace command string */
	    return(TRUE);		/* did an immediate command */
	}
	/* fall through to ignore character */
    default:
	uCch(c);			/* unget char */
	return(FALSE);			/* didn't do immediate */
    }
}
/* rdcmd1 -- get the actual command line
 *
 * This routine actually gets a command line.  In case the command line
 * gets entirely erased, we return an appropriate flag code telling what
 * the reason was for exit.  In fact, there are three important reasons:
 *
 *	1.  We got a real command line
 *	2.  The entire command line got erased
 *	3.  User typed ^C, which erases the command line (and a second
 *	    one kills the program
 */

static int
rdcmd1(flag)
int flag;				/* flag from previous call, or 0 */
{
    void cecho();			/* declare functions */
    void ccrlf();
    void remc();
    void echoline();
    void echoall();
    BOOL erasechar();
    BOOL eraseline();
    void interr();

    int c;				/* next char */

/* Keep going until one of many conditions stops us */

    while (TRUE)
    {
	c = rCch();			/* read next char */


/* check on current 'flag' condition */

	switch (flag)
	{
	case CTRLC:			/* in this case, the buffer was
					 * previously zeroed and we are
					 * looking for a possible second
					 * ^C, meaning exit
					 */
	    if (c == CTRLC)
	    {
		cecho(c);		/* if ^C, echo it */
		ccrlf();		/* echo cr, lf to get to new line */
		Xecimmed("hk ek ex");	/* execute this string */
		interr("^C ^C returned"); /* shouldn't get back here */
	    }
	    break;			/* otherwise treat as normal char */
	
	case ESC:			/* got an ESC last time:  look for
					 * second, meaning end of command
					 */
	    if (c == ESC)
	    {
		cecho(c);		/* echo ESC */
		Addc(Q_cmd,ESC);	/* add to command buffer */
		ccrlf();		/* do cr, lf */
		return(GOTCMD);		/* say we got a command */
	    }
	    break;			/* otherwise treat as normal */
	

	case BEL:			/* if last was BEL we have several
					 * possibilities
					 */
	    switch (c)
	    {
	    case BEL:			/* 2 BELs mean kill command */
		cecho(c);		/* echo BEL */
		ccrlf();		/* do cr, lf */
		return(DELETED);	/* indicate everything was deleted */
	    case SPACE:			/* means print current line */
		remc();			/* remove the BEL */
		ccrlf();		/* do cr, lf */
		echoline();		/* echo current line */
		flag = 0;		/* forget what we've seen */
		continue;		/* continue in loop */
	    case '*':			/* BEL * means print buffer */
		remc();			/* remove BEL from buffer */
		echoall();		/* echo the works */
		flag = 0;		/* forget what we've seen */
		continue;		/* continue scanning in loop */
	    default:			/* anything else */
		break;			/* nothing special */
	    };
	    break;			/* out of BEL case */
	default:			/* for main switch */
	    break;			/* out of it */
	};	/* end main switch */
/* for any character we haven't handled so far, throw away the current flag.
** There are two distinct cases here, depending on whether the input is coming
** from the terminal or not.
*/

	flag = 0;

	if (Inpsrc == SRC_TTY)		/* this switch for terminal chars */
	    switch (c)			/* now see if new char is special */
	    {
	    case CTRLC:			/* ^C */
		cecho(c);		/* echo char */
		ccrlf();		/* do cr, lf */
		return(CTRLC);		/* signify ^C seen */
	    case BEL:			/* BEL char */
	    case ESC:			/* ESC char */
		cecho(c);		/* echo char */
		Addc(Q_cmd,(char) c);	/* add to command buffer */
		flag = c;		/* remember char as flag */
		break;
	    default:			/* anything else (we can't do
					 * these in 'case' because they
					 * depend on variables
					 */
		if (c == Charkill)	/* is this char kill character? */
		{
		    if (erasechar())	/* if line all gone and not crt,
					** do CRLF
					*/
		    {
			if ((ET & ET_crt) == 0)
			    ccrlf();
			else
			    Eraseline(); /* clean out line for new prompt */
			return(DELETED);
		    }
		}
					/* delete char, return if all gone */
		else if (c == Linekill)	/* check line kill char */
		{
		    cecho(c);		/* echo the char */
		    if (eraseline()) return(DELETED);
		}
					/* same idea here */
		else
		{
		    cecho(c);		/* otherwise, echo the char */
		    Addc(Q_cmd,(char) c); /* add char to buffer */
		}
	    break;
	    }	/* end terminal character switch */
	else				/* non-terminal command char */
	{
	    if (c == ESC)		/* set flag on ESCape */
		flag = ESC;
	    
	    Addc(Q_cmd,(char) c);	/* add character to buffer */
	}	/* end else:  non-terminal character handling */
    };		/* end main while */
/*    interr("fell out of while in rdcmd1");	/* suppress compiler wrng. */
/*NOTREACHED*/
}
 /* Beginning of service routines for above routines */


/* cecho -- echo char, subject to input source
 *
 * This routine echoes a command character if it came from the
 * user's terminal
 */

static void
cecho(c)
int c;					/* char to echo */
{
    if (Inpsrc == SRC_TTY)
	wCch(c,Ttyout);			/* write to tty output */
    return;
}


/* ccrlf -- echo cr, lf, subject to EI flag */

static void
ccrlf()
{
    cecho(CR);
    cecho(LF);
    return;
}
/* remc -- unconditionally remove a char from buffer
 *
 * This routine blindly assumes that there is a character in the
 * text buffer to erase.
 */

static void
remc()
{
    TBdel(Q_cmd,TBsize(Q_cmd)-1,1);	/* delete one char at end */
    return;
}


/* doprompt -- output prompt char */

static void
doprompt()
{
    if (whCch() == SRC_TTY)		/* if next command char from tty */
    {
	ET &= ~ET_mung;			/* turn off exit on error bit */
	Dovflag(EV);			/* display text around '.' */
	Ttenable();			/* enable changes to terminal modes */

	Ttscmd();			/* set terminal modes for reading
					** from terminal
					*/
	wCch('*',Ttyout);		/* output prompt character */
    }
    return;
}
/* other service routines that should probably be moved at some point */

/* erasechar -- erase a char from buffer
 *
 * This routine removes the last character from the buffer and does the
 * appropriate terminal management
 */

static BOOL				/* TRUE if entire buffer gone */
erasechar()
{
    int last = TBsize(Q_cmd) - 1;	/* last position in buffer */
    char c;				/* character erased */
    void Erasechar();

    if (last >= 0)			/* make sure something's there */
    {
	c = *(TBtext(Q_cmd)+last);	/* get last char */
	TBdel(Q_cmd,last,1);		/* then delete it */
	Erasechar(c);			/* do erase output */
    }
    return(last <= 0);			/* TRUE if no characters left */
}


/* eraseline -- erase last line from buffer
 *
 * This routine removes the entire last line from the command buffer and
 * does the appropriate terminal management
 */

static BOOL				/* TRUE if only line in buffer
					 * was deleted
					 */
eraseline()
{
    int high = TBsize(Q_cmd) - 1;	/* position of last char */
    int low = Findlt(Q_cmd,high,0);	/* find LF preceding where we are */
    void Eraseline();

/* 'low' is 0 if there is no line terminator in the buffer.  Note that
** this will yield the decent values in the next line, even if the
** command text block is empty.
*/

    TBdel(Q_cmd,low,high-low+1);	/* delete chars from after LF to
					 * end of buffer
					 */
    Eraseline();			/* do erase-line output */
    return(low == 0);			/* deleted all if no line term. */
}
/* echoline -- echo the current line
 *
 * This routine echoes the current command line from the most recent LF
 * to the end of the buffer.
 */

static void
echoline()
{
    int high = TBsize(Q_cmd);		/* highest position */
    int first; 				/* first position to write */

    if (high > 0)
    {
	first = Findlt(Q_cmd,high-1,0);	/* find preceding line term. */
	wTBst(Q_cmd,first,high-first,Ttyout); /* write string from preceding
					 * lf to end
					 */
    }
    return;
}


/* echoall -- echo entire command buffer
 *
 * This routine echoes the entire command buffer
 */

static void
echoall()
{
    ccrlf();				/* do cr, lf */

    wTBst(Q_cmd,0,TBsize(Q_cmd),Ttyout); /* write entire string */
    return;
}
