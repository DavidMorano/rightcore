static char SCCSID[] = "@(#) cmdel.c:  6.1 12/19/83";
/* cmdel.c
 *
 *	TECO deletion commands
 *
 *	David Kristol, February, 1982
 *
 *	This module contains the D and K commands.
 */

#include "bool.h"
#include "cmutil.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "values.h"
#include "xec.h"
/* delspan -- delete span of characters
**
** This routine deletes a group of characters, starting at some defined
** position.  We also adjust '.' to equal the lower position of the
** starting position or that position plus the length, whichever is
** smaller.  This gives the correct behavior for deletes relative to
** '.', like -10d and 3k, as well as absolute deletes like 5,20d,
** for which the TECO manual dictates that '.' gets set to the first value.
** If the number of characters to be deleted is negative,
** we assume we are deleting in the - direction and act accordingly.
** As a service to the caller, numeric flags and values are discarded.
*/

static void
delspan(first,len)
int first;				/* position to start deleting at */
int len;				/* number of characters to delete */
{
    if (len < 0)			/* make things positive */
    {
	len = -len;
	first -= len;			/* set 'first' to lowest numbered
					** position
					*/
    }

    if (len != 0)			/* for zero length, nothing to do */
	TBdel(Q_text,first,len);	/* otherwise, do the deletion */

    Dot = first;			/* set new value of '.' */

    Eat_val();				/* discard flags and values
					** for caller
					*/
    Eat_flags();
    return;
}
/* D command
**
** We handle these forms:
**
**	nD	delete to right of '.'
**	-nD	delete to left of '.'
**	m,nD	same as m,nK:  delete from position m to n
*/

short
CMdcmd()
{
    int first;				/* starting position for deletion */
    int len;				/* size of deletion */
    int n;				/* temporary second part of m,nD */

    if (Skiptype != SKNONE)
	return(SKIPCONT);		/* exit if skipping */

    if (Get2val(&first,&n))		/* check for two values first */
    {
	Postest(first);			/* make sure both extremes lie
					** withing the buffer
					*/
	Postest(n);
	len = n - first;		/* set length to delete */
    }
    else
    {
	Set1dfl(1);			/* default is 1 */
	(void) Get1val(&len);		/* get length */

	first = Dot;			/* set initial position */
	Postest(first+len);		/* check that we're in bounds */
    }
/* first and len are set up for delspan */

    delspan(first,len);			/* delete the selected characters */
    return(XECCONT);
}
/* K command */

short
CMkcmd()
{
    int first;				/* first position for kill */
    int len;				/* length of kill */

    if (Skiptype == SKNONE)		/* if we're not skipping */
    {
	Confine(1,&first,&len);		/* default = 1, get bounds */

	delspan(first,len);		/* delete stuff within this space */
    }
    return(CONTINUE);
}
