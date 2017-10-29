static char SCCSID[] = "@(#) errors.c:  6.1 8/25/83";
/* errors.c
 *
 *	TECO error and trace handler
 *
 *	David Kristol, February, 1982
 *
 *	Error handling in TECO is primitive.  We print a message and
 *	jump to a reset location in the main loop.  Traces consist of
 *	printing the current command string up to the current position.
 *
 *	There are five kinds of error messages and corresponding routines:
 *
 *	1.  Plain string (terrNUL) (ErrNUL)
 *	2.  String with embedded character (terrCHR) (ErrCHR)
 *	3.  String with embedded text block content (terrTB) (ErrTB)
 *	4.  String with embedded other string (terrSTR) (ErrSTR)
 *	5.  Warning (terrWRN) (ErrWRN)
 *	6.  Special warning (terrWRN) (ErrSWRN)
 *
 *	Warnings do not cancel execution.
 *
 *	The first part of an error message always appears.  If the
 *	EH flag is 1, we stop there.  Otherwise we produce the
 *	expanded part of the message, too.  (We don't have a "Ward
 *	and Peace" mode, but it would be easy to put in.)
 *	Each message is followed by a crlf.  All messages go to
 *	'stderr'.
 */


#include <setjmp.h>			/* define longjmp stuff */
#include <stdio.h>
#include "bool.h"			/* boolean defs */
#include "chars.h"			/* character definitions */
#include "cmdio.h"			/* command i/o */
#include "crt.h"			/* crt handling */
#include "errors.h"			/* define structures */
#include "exittype.h"
#include "tb.h"				/* text block defs */
#include "tflags.h"
#include "tty.h"
#include "xec.h"



/* external variables */

extern int	EH ;			/* HELP level */
extern int	ET ;			/* TECO ET flag */


/* local variables */

BOOL Had_error = FALSE;			/* becomes TRUE if an error occurs */


/* define some basic service routines */

static void
wEch(c)					/* write char on standard error */
int c;
{
    wCch(c,stderr);
    return;
}

/* errout -- output error string, look for escape
 *
 * This routine outputs an error string.  If it finds an escape
 * character (presently %), it calls another routine to output
 * more characters.  Then it continues with the original string.
 */

static void
errout(st,rout)
char * st;				/* basic error string */
void (*rout)();				/* pointer to routine for escape */
{
    int c;				/* each character */

    while ((c = *st++) != 0)
	if (c == '%')			/* if escape... */
	    rout();			/* call escape routine */
	else
	    wEch(c);			/* else, output char */
    return;
}

static void
RNULL()					/* dummy routine for routines below */
{
    void interr();
    interr("RNULL called");
}


/* doerr -- do error beginning
 *
 * This routine does the standard stuff for error types 1 - 4.  At
 * prints the short form message and tests whether the long form is
 * appropriate.  If not it does an error exit.  If so, it returns a
 * pointer to the error string.
 * doerr also checks to be sure we have a match between error message
 * type and the called routine.
 */

static char *
doerr(err,type)
TERPTR err;				/* error pointer */
int type;				/* expecter error type */
{
    void errexit();
    void interr();
    void Killei();

    Killei();				/* kill EI file */

#ifdef DEBUG
    if (err->type != type)		/* double check type */
	interr("type mismatch in doerr");
#endif

    wEch(CR);				/* write CR, LF */
    wEch(LF);

    Crtstandout(TRUE);			/* turn on CRT standout mode */
    wEch('?');				/* write ? */
    errout(err->first,RNULL);		/* output first part */
    if ((EH & EH_type) == EH_short)	/* if short mode */
	errexit();			/* exit now */
    wEch(SPACE);			/* write preparatory space for
					 * second part
					 */
    return(err->second);		/* otherwise, return longer string */
}
/* errexit -- effect error exit */

static void
errexit()
{
/* declare the longjmp target */

    extern jmp_buf Reset;		/* place to go to in main code */
    extern void longjmp();
    void Dotrace();
    void Tecexit();

    Had_error = TRUE;			/* say we've had an error */

    Crtstandout(FALSE);			/* turn off CRT standout mode */
    wEch(CR);				/* write cr, lf */
    wEch(LF);

    if ((ET & ET_mung) != 0)		/* exit TECO on error if set */
	Tecexit(UEXITMUNG);		/* signal we had mung error, exit */
    
    if ((EH & EH_trace) != 0)		/* do trace if enabled */
	Dotrace();

/* reset ET bits */

    ET &= ~(ET_image | ET_rnoe | ET_rnow | ET_ctC);

    longjmp(Reset,ERROREXIT);		/* return to main loop */
}
/* These are the error handlers proper */

/* terrNUL -- type 1:  just error string */

void
terrNUL(err)
TERPTR err;				/* error pointer */
{
    errout(doerr(err,ErrNUL),RNULL);	/* output first part, then second */
    errexit();				/* do error exit */
}

/* terrCHR -- type 2:  with character */

static int Errchar;			/* error character -- shared by
					 * terrCHR and terr2rout
					 */
void
terrCHR(err,errchar)
TERPTR err;				/* error pointer */
int errchar;				/* the error character */
{
    void terr2rout();

    Errchar = errchar;			/* make accessible to terr2rout */
    errout(doerr(err,ErrCHR),terr2rout);
					/* output both parts, char */
    errexit();
}

static void
terr2rout()				/* helper routine for terrCHR */
{
    wEch(Errchar);			/* write the saved char */
    return;
}

/* terrTB -- type 3 error:  text block */

void
terrTB(err,tb)
TERPTR err;				/* error pointer */
int tb;					/* text block number */
{
    void terr34();

    terr34(err,TBtext(tb),TBsize(tb),ErrTB);
					/* treat as special case of string */
}
/* terrSTR -- type 4 error:  with string */

void
terrSTR(err,ptr,len)
TERPTR err;				/* pointer to error */
char * ptr;				/* pointer to other character string */
int len;				/* length of string */
{
    void terr34();

    terr34(err,ptr,len,ErrSTR);		/* call lower routine */
}


static char * Errst;			/* to share with terr4rout */
static int Errlen;

static void
terr34(err,ptr,len,type)
TERPTR err;				/* pointer to error */
char * ptr;				/* other string */
int len;				/* length of other string */
int type;				/* message type expected */
{
    void terr4rout();

    Errst = ptr;			/* make accessible to terr4rout */
    Errlen = len;
    errout(doerr(err,type),terr4rout); /* output first part, second, string */
    errexit();
}

static void
terr4rout()				/* helper for above:  output string */
{
    while (Errlen-- > 0)
	wEch(*Errst++);
    return;
}
/* terrWRN -- write warning message */

void
terrWRN(err)
TERPTR err;				/* pointer to error */
{
    char * st = err->first;		/* get string pointer */
    int c;				/* character */

#ifdef DEBUG
    if (err->type != ErrWRN && err->type != ErrSWRN) /* check for right type */
	interr("not warning type in terrWRN");
#endif

    Crtstandout(TRUE);			/* turn on CRT standout mode */
    wEch('%');				/* write % */
    wEch(SPACE);			/* write SPACE */

    while ((c = *st++) != 0)
	wEch(c);

    Crtstandout(FALSE);			/* turn off CRT standout mode */

    if (err->type == ErrSWRN)
	errexit();			/* on special, do normal error exit,
					** which picks up CRLF
					*/

    wEch(CR);				/* write CR, LF */
    wEch(LF);
    return;				/* return on normal warnings */
}
/* Dotrace -- print error trace back
**
** Dotrace produces an error trace on the user's terminal from the
** beginning of the current command string to the current command
** position.
*/

void
Dotrace()
{
    wCch('?',Ttyout);			/* start with ? */

    wTBst(Cmdtb,0,Cmddot,Ttyout);	/* print part of current text block
					** from beginning to current position
					*/
    wCch('?',Ttyout);			/* append '?' */

    Crlf(Ttyout);			/* end with cr/lf */
    return;				/* done */
}
