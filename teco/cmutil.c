static char SCCSID[] = "@(#) cmutil.c:  4.1 12/12/82";
/* cmutil.c
 *
 *	TECO command utilities
 *
 *	David Kristol, February, 1982
 *
 *	This module contains the following utilities:
 *
 *	Postest	-- test for potential bad position of ., output message
 *	Confine -- handle 0/1/2 arg commands (like K), confine positions
 *		   within text buffer
 *	Buildstring --
 *		   build "search string" using string building contstructs
 *	Addc --	append character to text block, generate error on failure
 *	Addst -- same as Addc, but for string
 */

#include "bool.h"
#include "chars.h"
#include "ctype.h"
#include "errors.h"
#include "errmsg.h"
#include "qname.h"
#include "tb.h"
#include "tflags.h"
#include "values.h"
#include "xec.h"
/* Postest -- test position
 *
 * This routine makes sure that the argument is within the bounds of the
 * text buffer.  If not, an error results
 */

void
Postest(pos)
int pos;				/* position to test */
{
    char c = Cmdchar;			/* save char in (char) for error */

    if (pos < 0 || pos > TBsize(Q_text))
	terrSTR(&Err_POP,&c,1);		/* pointer off page */
    return;
}
/* Confine -- confine positions within text buffer
 *
 * This routine supports commands like K that take 0, 1, or 2 args.
 * It confines the result of defaulting to the legal text buffer
 * extremes and produces an error for illegal values.  Note that too
 * large a (single) value for K, T, etc., stops at the buffer
 * boundaries automatically (see Findlt).
 */

void
Confine(dfl,first,len)
int dfl;				/* default to pass to set012pos */
TNUMB * first;				/* resulting first position */
int * len;				/* resulting difference between
					 * first and second numbers
					 */
{
    TNUMB lfirst;			/* (local) first position */
    TNUMB last;				/* last position */

    Set012dfl(dfl);			/* set default */

    (void) Get2val(&lfirst,&last);	/* get the values */

    Postest(lfirst);			/* check first value */
    Postest(last);			/* check second value */

/* both values are within the buffer confines.  Pass them back in order,
 * smaller first, as TECO-11 does.  Compute the length directly.
 */

    if (lfirst <= last)
    {
	*first = lfirst;		/* copy first */
	*len = last - lfirst;		/* compute + length */
    }
    else
    {
	*first = last;			/* reverse order here */
	*len = last - lfirst;		/* and reverse length calc. */
    }

    return;
}
/* Buildstring -- build "search string"
 *
 * This routine builds a "search string", which is mis-named because it is
 * also used in goto's and filename commands.  The string is built into a
 * designated text block.  The constructs recognized are:
 *
 *	^Q, ^R	next character is literal.  For search strings,
 *		keepquote = TRUE, so quoting can be seen again.  For
 *		other uses, keepquote=FALSE and the quote construct
 *		is replaced by the character quoted.
 *	^V, ^W	next character to be taken as lower (upper) case, if
 *		alphabetic
 *	^EQq	insert text portion of Q register q here
 *	^EUq	insert single character here whose code is in numeric
 *		part of Q register q
 *	^	designates up-arrow form of control characters, i.e., the
 *		two character sequence ^ E means ^E (one character)
 *
 * Because we can use the contents of two of the text blocks (search and
 * filename) in this structure, we first build the "search string" into
 * a temporary text block, then "rename" it to be the desired result tb.
 */

/* The build operation is state-driven.  The states are: */

#define NEWCONST	-1	/* get a new character, part of new
				 * construct
				 */
#define GOTCHAR		-2	/* a previous construct provided a character */
#define REPLACE		-3	/* replace previous construct with char */
#define CTRLEQ		-4	/* saw ^EQ */
#define CTRLEU		-5	/* saw ^EU */
#define EXIT		-6	/* exit from processing */

/* There are also a states corresponding to seeing '^', ^E, ^Q, ^R, ^V, and ^W.
** The respective state values are '^', CTRLE, CTRLQ, CTRLR,
** CTRLV, and CTRLW. */
void
Buildstring(tb,pos,len,keepquote)
short tb;				/* text block to build string into */
int pos;				/* first position of argument string
					 * in current command tb
					 */
int len;				/* length of argument string */
BOOL keepquote;				/* TRUE to leave quote constructs
					** in string
					*/
{
    char c;				/* next argument character */
    int ic;				/* int version of a character */

    short state = NEWCONST;		/* start off anticipating new
					 * construct
					 */
    short oldstate;			/* state at start of dispatch */
    short size;				/* size of current construct so far */
    int first;				/* position of first char in
					 * current construct
					 */

    short qname;			/* Q register name */
    TNUMB Getqval();
    extern TNUMB ED;			/* TECO ED flag */
    void interr();
    short Qnum();



    TBkill(Q_btemp);			/* delete all text in target text
					 * block
					 */
/* The processing simulates a simple FSA.  The current value of 'state'
 * tells where we are.
 */

    while (state != EXIT)
    {

/* first do state-dependent initializations */

	switch (state)
	{
	case GOTCHAR:			/* got constructed char */
	case REPLACE:			/* ... or replace a char */
	    break;			/* don't do anything */

	case NEWCONST:			/* start of new build construct */
	    size = 0;			/* no size of construct yet */
	    first = TBsize(Q_btemp);	/* initial pos. is current tb size */
	    /* fall through to get next character */
	
	default:			/* all other cases:  get next char,
					 * append.  We remove unwanted
					 * characters after we decide they
					 * are unwanted
					 */
	    if (len-- <= 0)		/* more chars available? */
		if (state == '^')	/* error if expecting successor to ^ */
		    terrNUL(&Err_ISS);	/* tried to read through string
					** terminator
					*/
		else
		{
		    state = EXIT;	/* this is loop exit condition */
		    continue;		/* get right to the loop test */
		}
	    
	    c = *(TBtext(Cmdtb) + (pos++)); /* get next char */
	    if (!TBapp(Q_btemp,1,&c))	/* always append it to built string */
		terrNUL(&Err_MEM);	/* out of space */
	    size++;			/* count one more char in construct */
	}
/* Now dispatch to proper processing routine depending on state */

	oldstate = state;		/* save current state */
	state = NEWCONST;		/* assume default new state */

	switch (oldstate)
	{
	case NEWCONST:			/* start of new construct */
	case GOTCHAR:			/* could be start of construct built
					 * from ^
					 */
	    switch (c)			/* depends on new character just read */
	    {
		case '^':		/* possible ^x construction */
		    if ((ED & ED_upar) != 0)
					/* is ^ literal or part of ^x? */
			break;		/* if bit set, literal.  It's already
					** been added to string
					*/
		case CTRLE:		/* saw ^E */
		case CTRLQ:		/* saw ^Q */
		case CTRLR:		/* saw ^R */
		case CTRLV:		/* saw ^V */
		case CTRLW:		/* saw ^W */
		    state = c;		/* just change state */
		    break;
		
		/* for all other cases, initialization has taken care of
		 * everything:  the character has been added to the built
		 * string.  Also the state is now (again) NEWCONST.
		 */
	    }
	    break;
	
	case '^':			/* character following ^ */
	    if ((ic = TRCTRL(c)) <0)	/* valid ^x construction? */
		terrCHR(&Err_IUC,ic);	/* no, bad 'x' */
	    c = ic;			/* make character version of char */
	    if (!TBrepl(Q_btemp,first,size,&c,1))
					/* replace ^ x by ^x (1 char) */
		interr("replace (1) failed in Buildstring");
	    
	    size = 1;			/* new size of current construct */
	    /* 'first' stays the same */
	    state = GOTCHAR;		/* have new char to examine */
	    break;
/* more cases */

	case CTRLE:			/* char following ^E */
	    if (c == 'q' || c == 'Q')
		state = CTRLEQ;		/* one possibility */
	    else if (c == 'u' || c == 'U')
		state = CTRLEU;		/* other possibility */
	    /* otherwise we leave ^Ex in built string */
	    break;
	
	case CTRLQ:			/* char following ^Q or ^R */
	case CTRLR:
	    if (keepquote)
		state = NEWCONST;	/* leave construct alone */
	    else
		state = REPLACE;	/* replace with the quoted char */
	    break;
	case CTRLV:			/* char following ^V */
	    c = LOWER(c);		/* get lower case equiv. or same char */
	    state = REPLACE;		/* replace current construct with
					 * resulting char.
					 */
	    break;

	case CTRLW:			/* char following ^W */
	    c = UPPER(c);		/* get upper case char. or same char */
	    state = REPLACE;		/* similar to above */
	    break;

	case REPLACE:			/* replace current construct by c */
	    if (!TBrepl(Q_btemp,first,size,&c,1))
		interr("replace (2) failed in Buildstring");
	    break;
/* ^Ex constructs:  we have in hand the character following ^Ex */

	case CTRLEQ:			/* char after ^EQ */
	    if ((qname = Qnum(c,TRUE)) < 0) /* allow special names, test for
					     * good name
					     */
		terrCHR(&Err_IQN,c);	/* bad name */
	    
	    /* replace construct by Q register content */

	    if (!TBrepl(Q_btemp,first,size,TBtext(qname),TBsize(qname)))
		terrNUL(&Err_MEM);	/* out of space if failed */
	    break;

	case CTRLEU:			/* char following ^EU */
	    if ((qname = Qnum(c,FALSE)) < 0) /* specials not allowed */
		terrCHR(&Err_IQN,c);	/* illegal Q name */
	    
	    c = Getqval(qname) & 0177;	/* make char of Q numeric value */
	    state = REPLACE;		/* replace current construct */
	    break;
	
/* pick up stray states for debugging */

#ifdef DEBUG
	default:
	    interr("bad state in Buildstring");
#endif
	} /* end state dispatch */
    
    } /* end while */

    TBrename(Q_btemp,tb);		/* rename our temporary text block
					** to be the desired tb
					*/
    return;
}
/* Addst -- append string to text block
**
** This routine appends a character string to a text block.  It makes
** two tries, if necessary, with a text block reorganization in between.
** If both tries fail, it generates a memory overflow error.
**
** NOTE:  It's essential that the string passed in here is not
** part of a text block.  The reason is that if the text block
** space is reorganized, the pointer provided might no longer
** point at the text block it did upon entry to the routine.
** For such situations a routine similar to this must be coded
** in line with explicit calls to TBtext and TBsize.
*/

void
Addst(tb,str,len)
short tb;				/* text block number */
char * str;				/* string to append */
int len;				/* length of string to append */
{
    if (TBapp(tb,len,str))		/* if append succeeds, exit */
	return;
    
    TBreorg();				/* otherwise, reorganize things */

    if (TBapp(tb,len,str))		/* try again */
	return;				/* success */
    
    terrNUL(&Err_MEM);			/* failed */
/*NOTREACHED*/
}
/* Addc -- append character to text block
**
** This routine essentially does the same thing as the above (because
** it calls it!).  The precautions above don't apply here because
** the character to be added has already been lifted from the text
** block area.
*/

void
Addc(tb,c)
short tb;				/* text block to append to */
char c;					/* character to append */
{
    Addst(tb,&c,1);			/* use routine above */
    return;
}
