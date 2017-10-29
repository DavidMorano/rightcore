static char SCCSID[] = "@(#) find.c:  4.1 12/12/82";
/* find.c
 *
 *	TECO character find routines
 *
 *	David Kristol, February, 1982
 *
 *	This module contains routines that are useful for searching for
 *	a single character.  They are:
 *
 *	Finddlm -- find delimiter, return start and span
 *	Find2dlm -- find 2 delimited strings, find start and span of both
 *	Findlt -- find line terminator, either forward or backward
 *	Findslt -- find line terminator forward in string
 */

#include "bool.h"
#include "chars.h"			/* define character names */
#include "ctype.h"			/* define character types */
#include "tb.h"				/* text block definitions */
#include "xec.h"			/* execution definitions */
/* Finddlm -- find delimiter
 *
 * This routine is useful for finding the delimiter for various commands.
 * It assumes that the command uses the @ convention, and that the next
 * command character will be the delimiter if Fl_at is set.  We signal an
 * unterminated command if we fail to find the closing delimiter.
 * Fl_at is unconditionally set FALSE.
 */


static int	Delim;			/* delimiter character, shared by
					** Finddlm and Find2dlm
					*/

void
Finddlm(delim,firstp,lenp)
int delim;				/* default delimiter (usually ESC) */
int * firstp;				/* pointer to place to put initial
					 * position of actual string
					 */
int * lenp;				/* pointer to place to put length
					 * of actual string
					 */
{
    int c;				/* temporary char */

/* check for @, get alternate delimiter */

    Delim = delim;			/* put delimiter where it can be
					** shared
					*/

    if (Fl_at && (Delim = gCMch()) < 0)	/* run off end ? */
	Unterm();			/* do unterminated command error */

    Fl_at = FALSE;			/* turn off flag */
    *lenp = 0;				/* initialize length and start */
    *firstp = Cmddot;
    while ((c = gCMch()) >= 0 && (c != Delim))
	(*lenp)++;			/* bump length of delimited string */
    
    if (c < 0)				/* check for running off end */
	Unterm();			/* issue appropriate error */
    return;
}
/* Find2ldm -- find 2 delimited strings
 *
 * This routine finds two delimited strings that share the same delimiter.
 */

void
Find2dlm(c,pfirst1,plen1,pfirst2,plen2)
char c;					/* default delimiter char */
int * pfirst1;				/* ptr to position of first string */
int * plen1;				/* ptr to length of first string */
int * pfirst2;				/* ptr to pos. of second string */
int * plen2;				/* ptr to length of second string */
{
    Finddlm(c,pfirst1,plen1);		/* delimit first string */

/* We assume here that Finddlm leaves its delimiter in Delim. */


    Finddlm(Delim,pfirst2,plen2);	/* delimit second string */
    return;
}
/* Findlt -- find line terminator
 *
 * This routine finds line terminators, which are FF, LF, VT.
 * As above, it returns the position of the character following the
 * one that was found.  However, Findlt treats text block boundaries
 * as line terminators.
 */

int					/* buffer position */
Findlt(tb,pos,iter)
int tb;					/* text block number */
int pos;				/* starting position */
int iter;				/* number of terminators to find */
{
    register char * cp = TBtext(tb) + pos;
					/* starting position for search */
    int count;				/* number of positions to examine */

    if (TBsize(tb) <= 0)		/* if empty text block, return 0 */
	return(0);


    if (iter > 0)			/* forward search */
    {
	count = TBsize(tb) - pos;	/* number of chars to look at */
	while (--count >= 0 && iter > 0)
	{
	    if (ISLT(*cp))		/* decr. count if line terminator */
		iter--;
	    cp++;
	}
/* note that in all cases when we fall through cp is pointing at the
 * char just past the last one we were looking for
 */

	return(cp-TBtext(tb));		/* return position */
    }
    else				/* backward search */
    {
	count = pos + 1;		/* deal with position 0, too */
	/* note that iter = 0 means go backward 1 */

	while (--count >= 0 && iter <= 0)
	{
	    --cp;			/* bump back ptr */
	    if (ISLT(*cp))		/* bump iteration if line terminator */
		iter++;			/* saw one more */
	}

/* on loop exit we have two cases to contend with:
 *	1.  if the iterations are exhausted, we succeeded but the pointer
 *	    is off by one:  we want to point
 *	    to the char that followed the one we found
 *	2.  otherwise we ran off the front of the tb:  return 0
 */

	return((iter > 0 ? cp-TBtext(tb)+1 : 0));
    }
}
/* Findslt -- find line terminator forward in string
**
** This routine scans a string for a line terminator.  It returns
** a pointer to the found terminator, or NULL.
*/

char *
Findslt(s,n)
register char * s;			/* pointer to string */
int n;					/* number of chars to scan */
{
    while (n-- > 0)
    {
	if (ISLT(*s))			/* find one? */
	    return(s);			/* yes */
	s++;				/* no.  Bump pointer */
    }
    return((char *) 0);			/* failed */
}
