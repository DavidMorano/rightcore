static char SCCSID[] = "@(#) cmins.c:  4.1 12/12/82";
/* cmins.c
 *
 *	TECO insert commands
 *
 *	David Kristol, February, 1982
 *
 *	This module contains the TECO I, TAB, and FR commands.
 */

#include "bool.h"
#include "chars.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "find.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "values.h"
#include "xec.h"
/* doins -- do the insertion
 *
 * This routine does the hard work of an insert:  puts the text into
 * the text buffer, tries to work around running out of memory, bumps
 * . , and sets the insert length.  If the TAB flag is set, a TAB is
 * inserted just ahead of the string that's passed as an argument.
 * The inserted string is assumed to come from Cmdtb.
 */

void
doins(pos,len,flag)
int pos;				/* position in tb to start from */
int len;				/* length of insert */
BOOL flag;				/* TRUE to leave room for TAB */
{
    if (flag)				/* leave room for TAB? */
    {
	len++;				/* longer insert */
	pos--;				/* *** CROCK ***:
					** copy a dummy character (which must
					** be there, given what we know about
					** the callers) to leave room for TAB
					*/
    }

/* we want to give the user every chance of having a successful
 * insert.  Therefore we may reorganize the buffer if necessary
 */
    if (len > 0 && ! TBins(Q_text,Dot,len,TBtext(Cmdtb)+pos))
    {
	TBreorg();			/* try to reorganize */
	if (!TBins(Q_text,Dot,len,TBtext(Cmdtb)+pos))
					/* try again */
	    terrNUL(&Err_MEM);		/* if fail, give up */
    }

/* Stick in TAB if requested */

    if (flag)
	*(TBtext(Q_text)+Dot) = TAB;

    Dot += len;				/* point past insert */
    Inslen = len;			/* set insert length */
    return;
}
/* I command */

short
CMicmd()
{
/* remember that the I command can take a preceding argument and a
 * succeeding text argument, but not both
 */
    int start;				/* starting position in command tb */
    int len;				/* length of insert */
    int t;				/* preceding value */

    Finddlm(ESC,&start,&len);		/* usual delimiter is ESCape; find
					 * bounds of string
					 */

    if (Skiptype != SKNONE)
	return(SKIPCONT);		/* if skipping, return now */

    if (Get1val(&t))			/* if there's a value */
    {
	char c = t & 0177;		/* temporary char to point to */

	if (len != 0)
	    terrNUL(&Err_IIA);		/* illegal insert */

	if (!TBins(Q_text,Dot,1,&c))
	    terrNUL(&Err_MEM);		/* if fail, give error */
	Dot++;				/* skip over new insert */
	Inslen = 1;			/* set insert length */
    }
    else
	doins(start,len,FALSE);		/* else insert the string, no TAB */
    Eat_val();				/* kill values */
    Eat_flags();			/* kill flags */
    return(XECCONT);
}
/* ^I (TAB) command */

short
CMcticmd()
{
    int start;				/* start position of insert string */
    int len;				/* length of same */

    Finddlm(ESC,&start,&len);		/* delimit the string */

    if (Skiptype == SKNONE)		/* do insert if not skipping */
	doins(start,len,TRUE);		/* leave space for TAB, put it in */
    return(CONTINUE);
}
/* FR command -- replace (length of last insert/search/etc.) characters
** with argument string.
*/

short
CMfrcmd()
{
    int first;				/* initial position of replacement */
    int replen;				/* length of replacement */
    int errfirst = Cmddot - 2;		/* position in command string of
					** start of command
					*/

    Finddlm(ESC,&first,&replen);	/* delimit replacement string */

    if (Skiptype != SKNONE)		/* exit if skipping state */
	return(SKIPCONT);
    
/* There is a choice here about what to do if '.' is too close to the start
** of the buffer, so there aren't enough characters to replace.  We could
** announce an error or simply replace as many characters as we can.  We
** choose to do the former.
*/

    if (Inslen > Dot)			/* report error if bad '.' */
	terrSTR(&Err_POP,TBtext(Cmdtb)+errfirst,2);
    
    if (!TBrepl(Q_text, Dot-Inslen, Inslen, /* replace text in text buffer... */
		TBtext(Cmdtb)+first,
		replen))		/* with text in command string */
	terrNUL(&Err_MEM);		/* memory overflow on failure */
    
/* Now adjust '.' and most-recent-insert length */

    Dot = Dot - Inslen + replen;	/* '.' sits just after insertion */
    Inslen = replen;
    Eat_val();				/* discard flags and values */
    Eat_flags();
    return(XECCONT);
}
