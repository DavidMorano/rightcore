static char SCCSID[] = "@(#) cmtecio.c:  4.1 12/12/82";
/* cmtecio.c
 *
 *	TECO file input/output commands
 *
 *	David Kristol, March, 1982
 *
 * This module contains those TECO commands that do file input/output:
 *
 *	A P PW Y EY
 *
 * It also contains these service routines which are used elsewhere:
 *
 *	Dopcmd -- do the essential parts of the P command
 */

#include <stdio.h>
#include "bool.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "tflags.h"
#include "tty.h"
#include "values.h"
#include "xec.h"
/* declare some externals that are used universally herein */

extern TNUMB Fl_ctE;			/* form feed flag */

extern TNUMB Readlines();
extern void Writebuf();
extern TNUMB Doyank();
/* yhelper -- helper routine for Y and EY commands */

static short
yhelper(flag)
BOOL flag;				/* TRUE to do yank protection */
{
    TNUMB val;				/* returned yank value */

    if (Skiptype != SKNONE)		/* if skipping, return */
	return(SKIPCONT);
    
    if (Get1val(&val))			/* if there's a value, error */
	terrNUL(&Err_NYA);
    
    val = Doyank(flag);			/* do the basic yank, get value */

    Eat_val();				/* discard values */
    if (Fl_colon != 0)			/* if : flag set, set returned value */
	Set1val(val);
    Eat_flags();			/* discard flags */
    return(XECCONT);			/* continue execution */
}
/* Y and EY commands */

short
CMycmd()
{
    return(yhelper(TRUE));		/* do yank with protection */
}


short
CMeycmd()
{
    return(yhelper(FALSE));		/* do yank without protection */
}
/* Dopcmd -- service routine for P command
 *
 * This routine writes the entire text buffer, adds a form feed if
 * appropriate, then gets a new buffer.  It returns -1 or 0 as to
 * whether the read did not or did work.
 */

TNUMB
Dopcmd(ffflag)
BOOL ffflag;				/* TRUE to append FF to buffer */
{
    TNUMB Doyank();

    Writebuf(0,TBsize(Q_text),		/* whole buffer */
		((Fl_ctE != 0) | ffflag));/* append FF if ^E set or explicit
					 * request
					 */
    return(Doyank(FALSE));		/* yank without protection (since we
					 * just wrote the buffer)
					 */
}
/* P, PW command */

/* This command has several cases which are dealt with separately.  Basically
 * all of the PW cases are equivalent to the P cases with the addition of
 * writing a FF.  The cases are:
 *
 *	P	PW	no arg (possibly with : flag )
 *	nP	nPW	one arg
 *	m,nP	m,nPW	two args
 */

short
CMpcmd()
{
    BOOL pwflag = FALSE;		/* did not yet see PW */
    char c = pCMch();			/* peek at next character */
    TNUMB m,n;				/* 2 returned values */
    int len;				/* length of write for m,nPW */
    void Postest();

/* check for PW */

    if (c == 'w' || c == 'W')
    {
	pwflag = TRUE;			/* saw PW */
	(void) gCMch();			/* gobble the character */
    }

    if (Skiptype != SKNONE)		/* if skipping, continue */
	return(SKIPCONT);
/* do case analysis */

    if (Get2val(&m,&n))			/* if two values */
    {
	Postest(m); Postest(n);		/* make sure values are within bounds */
	if ((len = n - m) <= 0)		/* must be positive length */
	    len = 0;
	
	Writebuf(m,len,FALSE);		/* write, no FF for 2 arg version */

	Eat_val();			/* discard values and flags */
	Eat_flags();
    }
    else if (Get1val(&n))		/* have one value */
    {
	if (n <= 0)			/* must be > 0 */
	    terrNUL(&Err_NPA);
	
	while (n-- > 0)			/* do repetitive P(W) commands */
	{
	    if (pwflag)			/* depends on PW flag */
		Writebuf(0,TBsize(Q_text),TRUE);
					/* write whole buffer, force FF */
	    else
		(void) Dopcmd(FALSE);	/* write, get next, no forced FF */
	
	    Testinterrupt();		/* check for interrupt after each */
	}
	
	Eat_val();			/* discard values and flags */
	Eat_flags();
    }
    else				/* no values */
    {
	Eat_val();			/* discard vestigial expression */

	if (pwflag)			/* depends on whether PW or P */
	    Writebuf(0,TBsize(Q_text),TRUE);
	else				/* P */
	{
	    n = Dopcmd(FALSE);		/* don't force FF */

	    if (Fl_colon != 0)		/* set value if : precedes */
		Set1val(n);
	}

	Eat_flags();			/* now discard flags */
    }
    return(XECCONT);
}
/* A command 
 *
 * This command is a little bizarre, since there is an unrelated case
 * thrown in for good measure.  The various cases for A are:
 *
 *	A		append new buffer
 *	:A		append buffer, return success indicator
 *	n:A		append n lines to buffer, return success indicator
 *	nA		(the odd one) returns value of n-th character from
 *			. in the buffer
 */

short
CMacmd()
{
    TNUMB n;				/* argument value, if any */
    TNUMB val;				/* returned value from Readlines */
    void Postest();

    if (Skiptype != SKNONE)		/* continue if skipping */
	return(SKIPCONT);

    
    if (Get1val(&n))			/* get preceding value, if any */
    {
	if (Fl_colon == 0)		/* dispose of odd case */
	{
	    n += Dot;			/* desired position */
	    Eat_flags();		/* discard current flags */
	    if ( n < 0 || n >= TBsize(Q_text))
		Set1val((TNUMB) -1);	/* return -1 if out of bounds */
	    else
		Set1val(		/* set ASCII value of char */
		    (TNUMB) *(TBtext(Q_text) + n)
			);
	    return(XECCONT);
	}
    }
    else				/* no predecessor value */
	n = -1;				/* set -1 to read whole buffer */
    
/* now ready to read lines */

    val = Readlines(n,TRUE);		/* note we read a whole buffer for
					 * n <= 0 (see Readlines); force
					 * expansion
					 */
    Eat_val();				/* discard current values */
    if (Fl_colon != 0)			/* set value if : flag */
	Set1val(val);
    Eat_flags();			/* discard flags */
    return(XECCONT);
}
