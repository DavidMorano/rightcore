static char SCCSID[] = "@(#) cmpos.c:  4.1 12/12/82";
/* cmpos.c
 *
 *	TECO position related commands
 *
 *	David Kristol, February, 1982
 *
 *	This module contains these TECO commands:
 *
 *	B, H, Z, ., C, J, L, R
 */

#include "bool.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "find.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "values.h"
#include "xec.h"
/* moveto -- move Dot to a new position
 *
 * This routine is handy for some of the commands in this module.
 * It checks for a good value, then changes . .
 */

void
moveto(pos)
int pos;				/* position to move to */
{
    void Postest();

    Postest(pos);			/* check new position */
    
    Dot = pos;				/* set new position */
    Eat_val();				/* discard values */
    Eat_flags();			/* discard flags */
    return;
}
/* some simple values */

/* B command */

short
CMbcmd()
{
    if (Skiptype == SKNONE)		/* only if not skipping */
        Set1val(0);			/* B is synonym for 0 */
    return(CONTINUE);			/* not done executing */
}

/* H command */

short
CMhcmd()
{
    if (Skiptype == SKNONE)
        Set2val(0,TBsize(Q_text));	/* same as 0,Z */
    return(CONTINUE);
}

/* . command */

short
CMdotcmd()
{
    if (Skiptype == SKNONE)
        Set1val(Dot);			/* current position */
    return(CONTINUE);
}


/* Z command */

short
CMzcmd()
{
    if (Skiptype == SKNONE)
        Set1val(TBsize(Q_text));	/* = number of chars in buffer */
    return(CONTINUE);
}
/* Basic movement commands */

/* C command */

short
CMccmd()
{
    int t;				/* position incr. */

    if (Skiptype != SKNONE)
	return(SKIPCONT);

    Set1dfl(1);				/* set default of 1 */

    (void) Get1val(&t);			/* get a value */
    moveto(Dot+t);			/* try to set new position */
    return(XECCONT);
}

/* R command */

short
CMrcmd()
{
    int t;				/* position incr. */

    if (Skiptype != SKNONE)
	return(SKIPCONT);

    Set1dfl(1);				/* default is 1 */
    (void) Get1val(&t);			/* get the value */
    moveto(Dot-t);			/* move backwards */
    return(XECCONT);
}

/* J command */

short
CMjcmd()
{
    int t;				/* position incr. */

    if (Skiptype != SKNONE)
	return(SKIPCONT);

    Set1dfl(0);				/* default is 0 (i.e., 0J) */
    (void) Get1val(&t);			/* get the value */
    moveto(t);				/* set absolute position */
    return(XECCONT);
}
/* L command */

short
CMlcmd()
{
    int t;				/* line incr. */

    if (Skiptype != SKNONE)
	return(SKIPCONT);

    Set1dfl(1);				/* set default of 1 */
    (void) Get1val(&t);			/* get value */
    moveto(Findlt(Q_text,Dot,t));	/* move to proper line offset */
    return(XECCONT);
}
