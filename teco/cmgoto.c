static char SCCSID[] = "@(#) cmgoto.c:  4.2 1/7/83";
/* cmgoto.c
 *
 *	TECO goto handling
 *
 *	David Kristol, March, 1982
 *
 *	This module contains code to handle labels and gotos in TECO.
 *	The relevant commands are:
 *
 *	! O
 *
 */

#include "bool.h"
#include "chars.h"
#include "cmutil.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "find.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "values.h"
#include "xec.h"
#include "xectype.h"
/* ! command:  comment or label */

short
CMexcl()
{
    int strncmp();

    int first;				/* starting position of label */
    int len;				/* length of delimited string */

    char * labelp;			/* pointer to label string */
    char * gotop;			/* pointer to going-to string */

/* first, find end of label */

    Finddlm('!',&first,&len);		/* identify the label */
    Eat_flags();			/* kill off flags, leave value */

/* The only time labels interest us are when we are in a skip state
 * looking for a label (SKGOTO).  Otherwise we just exit.
 * The current label can only be the target label if their lengths match.
 */

    if ( ( Skiptype == SKGOTO) && ( len == TBsize(Q_goto) ) )
    {
	labelp = TBtext(Cmdtb) + first;	/* pointer to current label */
	gotop = TBtext(Q_goto);		/* pointer to going-to label */

	if (strncmp(labelp,gotop,len) == 0)
	    return(SKIPDONE);		/* found label:  done skipping */
    }
    return(CONTINUE);			/* otherwise, continue. */
}
/* goto command */

short
CMocmd()
{
    int findcomma();
    int first;				/* position of start of target */
    int len;				/* length of target name */
    int last;				/* position of last target
					** string char
					*/
    int next;				/* position after second , */
    int gotopos;			/* start position of target label */
    int gotolen;			/* length of target label string */

    TNUMB t;				/* argument to command */
    BOOL gotval;			/* got-value flag */

    extern int Iterpos;			/* start position of current
					 * iteration
					 */

    Finddlm(ESC,&first,&len);		/* find delimited target label */

/* only do something when we're not currently skipping */

    if (Skiptype != SKNONE)
	return(SKIPCONT);		/* continue skipping */
    
    last = first + len - 1;		/* position of last target char */

    gotval = Get1val(&t);		/* get value, set flag */
    Eat_val();				/* eat values and flags */
    Eat_flags();

    if (!gotval)			/* if value, is computed goto */
    {
	gotopos = first;		/* start of target label */
	gotolen = len;			/* length of target label */
    }

/* we have a value:  this is a computed goto.  We must find the n-th and
 * n + 1st commas that delimit the target.  n = 0 selects the first label.
 */

    else if ((gotopos = findcomma(first,last,t)) < 0)
					/* look for t-th comma */
	return(XECCONT);		/* no such label:  just fall through */

/* at this point gotopos is the position just after the desired comma.
 * We must find its successor, which might not be there if it is the last
 * possible target label.
 */

    else
    {
	next = findcomma(gotopos,last,1); /* find next comma after t-th */
	if (next < 0)			/* no second comma */
	    gotolen = last - gotopos + 1; /* goes to last position */
	else
	    gotolen = next - 1 - gotopos; /* -1 to discount second , */
    }

/* We now set things up to search for this label.  gotopos is the position
 * of the target label and gotolen is the target's length.
 */

/* First, build the target label, using string build operators */

    Buildstring(Q_goto,gotopos,gotolen,FALSE);
					/* build into GOTO text block,
					** translate quoted chars
					*/

/* We want to start searching for the label from the current "macro"
 * level, which includes iterations.  Macros begin at position 0.
 */

    Cmddot = (Xectype == XECITER ? Iterpos : 0);
    if (Skip(SKGOTO) != SKIPDONE)	/* try to complete goto */
	terrTB(&Err_TAG,Q_goto);	/* if not done, couldn't find label */
    return(XECCONT);			/* else, exit successfully */
}
/* findcomma -- find commas
 *
 * This routine finds the i-th comma between the two argument positions
 * and returns the position just after it, or -1 if not found
 */

int
findcomma(first,last,n)
int first;				/* first position to check */
int last;				/* last position to check */
int n;					/* number of commas to scan */
{
    char * beg = TBtext(Cmdtb);		/* beginning of command */
    char * cp = beg + first;		/* initial string pointer */
    char * limit = beg + last;		/* limiting string pointer */

    while (cp <= limit && n > 0)
	if (*cp++ == ',')
	    n--;			/* decrement count each time */
    return (n==0 ? cp - beg : -1);	/* return position or -1 */
}
