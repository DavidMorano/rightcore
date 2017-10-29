static char SCCSID[] = "@(#) cmmode.c:  6.1 8/25/83";
/* cmmode.c
**
**	TECO mode flag handler
**
**	David Kristol, April, 1982
**
** This module handles the TECO mode flags.  The general syntax for such
** commands is:
**
**	<flag name>			return mode flag value
**	n<flag name>			set mode flag to n
**	m,n<flag name>			clear 'm' bits, set 'n' bits in flag
**/

#include <stdio.h>			/* for tty.h */
#include "bool.h"
#include "crt.h"			/* for Iscrt */
#include "exittype.h"
#include "skiptype.h"
#include "tflags.h"
#include "tty.h"
#include "values.h"
#include "xec.h"



/* Declare the global mode flags */

TNUMB	ED;		/* ED flag:  edit level flag */
TNUMB	EH;		/* EH flag:  help level flag */
TNUMB	ES;		/* ES flag:  search verification flag */
TNUMB	ET;		/* ET flag:  terminal control flag */
TNUMB	EU;		/* EU flag:  upper/lower case flag */
TNUMB	EV;		/* EV flag:  edit-verify flag */

TNUMB	Fl_ctX;		/* ^X flag:  search mode flag */

extern	BOOL Fl_ctO;	/* ^O flag (elsewhere) */


/* domode -- do most of the work for these commands
**
** This routine does the core of the work for all of these flags, namely:
**
**	1.  Check for skipping state and exit appropriately.
**	2.  Check for 0, 1, or 2 supplied values and act accordingly.
*/

BOOL					/* TRUE if not skipping, and a
					** value was set
					*/
domode(pflag, nochange)
TNUMB * pflag;				/* pointer to flag */
int nochange;				/* bits not to be changed:  unchanged
					** if 1, changeable if 0 (bitwise)
					*/
{
    TNUMB off;				/* bits to turn off */
    TNUMB on;				/* bits to turn on */

    if (Skiptype != SKNONE)		/* check for skipping */
	return(FALSE);			/* and indicate so */
    
    if (Get2val(&off,&on))		/* if two values provided, nothing
					** further to do
					*/
	;
    else if (Get1val(&on))		/* try to get single value */
	off = ~0;			/* turn off all bits in this case */
    else				/* no values provided:  return one */
    {
	Set1val(*pflag);		/* set this value */
	return(FALSE);			/* and indicate no value set */
    }

/* Reaching here, off and on are set appropriately and we will set a
** new mode flag value.
*/

    off &= ~nochange;			/* don't turn these bits off */
    on  &= ~nochange;			/* ... or on */

    *pflag = (*pflag & ~off) | on;	/* turn selected bits off and on
					** Note that we have not changed
					** unchangeable bits.
					*/
    
    Eat_flags();			/* discard flags and values */
    Eat_val();
    return(TRUE);			/* not skipping */
}
/* Individual mode flag commands */

/* ED command */

short
CMedcmd()
{
    void SMsetexp();			/* set SM expansion flag */

    if (domode(&ED, 0))			/* no prohibited bits */
	SMsetexp((ED & ED_expand) != 0); /* turn off/on memory expansion
					 ** if not skipping
					 */
    return(CONTINUE);
}


/* EH flag */

short
CMehcmd()
{
    (void) domode(&EH, 0);		/* no prohibited bits */
    return(CONTINUE);			/* continue executing/skipping */
}


/* ES flag */

short
CMescmd()
{
    (void) domode(&ES, 0);		/* no prohibited bits */
    return(CONTINUE);
}
/* ET flag */

short
CMetcmd()
{
    if (domode(&ET, ET_crtW | ET_refW))	/* can't turn off W-cmd-related bits */
    {
    /* set various flags, based on new values of ET */

	if ( (ET & ET_ctO) != 0)	/* ^O cancel flag */
	{
	    Fl_ctO = FALSE;		/* turn off ^O flag (enable output) */
	    ET &= ~ET_ctO;		/* turn bit off again */
	}

	/* can't turn on ET_crt if terminal is not a CRT */

	if (! Iscrt)
	    ET &= ~ET_crt;

	/* set ^T read with/without wait */

	if ( (ET & ET_rnow) != 0)	/* read with no wait set */
	    Ttsxecnd();			/* set terminal for no wait */
	else
	    Ttsxec();			/* set for wait */
    }
    return(CONTINUE);			/* continue execution/skipping */
}


/* EU flag */

short
CMeucmd()
{
    (void) domode( &EU, 0);		/* no prohibited bits */
    return(CONTINUE);
}


/* EV flag */

short
CMevcmd()
{
    (void) domode( &EV, 0);		/* no prohibited bits */
    return(CONTINUE);
}


/* ^X flag */

short
CMctxcmd()
{
    (void) domode( &Fl_ctX, 0);		/* no prohibited bits */
    return(CONTINUE);
}
