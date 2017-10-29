static char SCCSID[] = "@(#) cmqreg.c:  4.1 12/12/82";
/* cmqreg.c
 *
 *	TECO Q register commands
 *
 *	David Kristol, March, 1982
 *
 *	This module contains TECO's Q-register commands:
 *
 *	X ^U G Q U [ ]
 */

#include	<sys/types.h>
#include	<string.h>
#include <stdio.h>

#include "bool.h"
#include "chars.h"
#include "ctype.h"
#include "cmutil.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "find.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "tty.h"
#include "values.h"
#include "xec.h"



/* external subroutines */

extern void	interr() ;



/* TECO maintains a Q register stack.  Each stack frame contains the
** text and numeric value from a Q register.  An additional value is
** carried along in each stack frame which is for bookkeeping purposes.
** The stack is maintained in one text block, with successive frames
** appended to the end.  For portability reasons numeric values are
** held on the stack in ASCII, rather than as contiguous bytes.  A
** stack frame looks like:
**
**	***-previous stack frame-***num:prev_sp:text
**
**	(top of stack ---> )
**
**	where 	"num"		is the ASCII-encoded numeric value from
**				the Q register
**		"prev_sp"	is the position at which the previous stack
**				frame began
**		"text"		is the Q register text contents
**
** Note that we can figure out the size of the text portion of the top-of-stack
** frame as long as we know where the text begins.  The code keeps track of
** the beginning of the current stack frame and always "pushes" the start of
** the previous one on the stack.


/* This array contains the Q register numeric values */

static TNUMB qvalue[Q_MAX+1];


/* first some service routines */

/* Qnum -- return text block number corresponding to a Q register name.
 *
 * This routine returns the text block number that corresponds to a
 * Q register name.  If the "name" is erroneous, the appropriate error
 * message is generated.
 */

short
Qnum(c,flag)
int c;				/* character to convert to tb number */
BOOL flag;			/* true if _ and * are valid names here */
{
    if (ISQREG(c,flag))		/* make sure it's valid Q-register name */
	return(QNUM(c));	/* then return Q-register number */
    else
	terrCHR(&Err_IQN,c);	/* otherwise, illegal name */
/*NOTREACHED*/
}
/* RdQnum -- read Q register name
 *
 * This routine reads the next command character and treats it as a
 * Q register name.  It returns the corresponding text block number.
 */

short
RdQnum(flag)
BOOL flag;			/* whether _ or * is a valid name */
{
    short c = gCMch();		/* get character */

    if (Skiptype != SKNONE)	/* if we're really skipping */
	return(-1);		/* return bum number */
    
    if (c < 0)			/* run off end ? */
	Unterm();		/* flag as error */
    
    return(Qnum(c,flag));	/* return result of conversion */
}
/* Getqval -- get Q register value
 *
 * This routine returns the value in a Q register's numeric area.  It
 * assumes the Q register number is valid.
 */

TNUMB
Getqval(q)
short q;			/* Q register number */
{
    return(qvalue[q]);
}
/* qins -- handle Q-register insertion
 *
 * This routine resembles doins in cmins:  it inserts (or appends) text
 * to a Q register.  It gives the user the best possible attempt:  if
 * a first insertion attempt fails, we try to reorganize the text block
 * space and insert again.  Only then do we give up.
 *
 * Note that Fl_colon dictates whether we append (non-0) or insert (0).
 */

static void
qins(qnum,tb,pos,len)
short qnum;			/* Q register text block number */
short tb;			/* text block to insert from */
int pos;			/* starting position to insert from */
int len;			/* number of characters to insert/append */
{
    if (Fl_colon == 0)		/* when inserting directly, delete current
				 * contents of target Q reg.
				 */
	TBkill(qnum);

/* note that we will append to a text block which we may have just emptied */

    if (!TBapp(qnum,len,TBtext(tb)+pos))
    {				/* make first attempt to insert text */
	TBreorg();		/* first attempt failed.  Reorganize */

/* note that we must use TBtext and a pos again because the text block
 * may have moved during reorganization.
 */
	if (!TBapp(qnum,len,TBtext(tb)+pos))
	    terrNUL(&Err_MEM);	/* second attempt failed.  Give up */
    }
    Eat_flags();		/* discard flags and values */
    Eat_val();
    return;
}
/* X command:  put text in Q-register */

short
CMxcmd()
{
    void Confine();

    short tb = RdQnum(FALSE);	/* get q register number; normal ones only */
    int first;			/* initial position to insert from */
    int len;			/* insertion length */

    if (Skiptype != SKNONE)
	return(SKIPCONT);	/* continue skipping */
    
    Confine(1,&first,&len);	/* set default, confine values to buffer
				 * bounds.
				 */

    qins(tb,Q_text,first,len);	/* insert from text buffer to q reg. */
    return(XECCONT);		/* continue executing */
}
/* ^U command:  put command text into Q-register */

short
CMctu()
{
    short tb = RdQnum(FALSE);	/* get Q reg number, normal only */
    int first;			/* initial position of insertion text */
    int len;			/* length of insertion text */
    TNUMB t;			/* numeric argument */
    char c;			/* single character for insertion/append */

    Finddlm(ESC,&first,&len);	/* span text argument, if any */

    if (Skiptype != SKNONE)
	return(SKIPCONT);	/* just continue skipping */
    
    if (! Get1val(&t))		/* get one value */
    {
	qins(tb,Cmdtb,first,len); /* if no value, use text from current cmd */
	return(XECCONT);	/* continue executing */
    }

    if (len != 0)		/* check for value and non-0 length */
	terrNUL(&Err_IIA);	/* same error as TECO-11:  illegal insert */
    
    c = ( (int) t) & 0177;	/* make real character from value */
    if (Fl_colon == 0)		/* kill Q register if not appending */
	TBkill(tb);
    
    Addc(tb,c);			/* append char to Q-reg */
    
    Eat_flags();		/* discard flags and values */
    Eat_val();
    return(XECCONT);		/* continue execution */
}
/* G command -- insert Q register into text buffer or print it */

short
CMgcmd()
{
    short tb = RdQnum(TRUE);	/* get Q reg., allow specials */
    void wTBst();

    if (Skiptype != SKNONE)
	return(SKIPCONT);	/* if skipping, continue */
    
    if (Fl_colon != 0)		/* print out ? */
	wTBst(tb,0,TBsize(tb),Cmdout); /* Yes.  Print entire Q register */
    else

/* once again we give the user two chances on an insertion */

    {
	int size = TBsize(tb);	/* size of text block we're inserting */

	if (! TBins(Q_text,Dot,size,TBtext(tb)+0))
	{
	    TBreorg();
	    if (! TBins(Q_text,Dot,size,TBtext(tb)+0))
		terrNUL(&Err_MEM); /* out of memory */
	}
	Dot += size;		/* skip over insertion */
	Inslen = size;		/* size of most recent insertion */
    }
    Eat_flags();		/* discard flags */
    return(XECCONT);		/* continue execution */
}
/* Q command -- get Q register value */
/* There are three formats:
 *	Qq	get numeric contents of Q register q
 *	:Qq	get size of text part of Q register q
 *	nQq	return value of nth character in Q register q or -1 if
 *		n is too large
 */
short
CMqcmd()
{
    short qnum;			/* Q-register number */
    TNUMB t;			/* current value, if any */
    TNUMB retval;		/* value to return */

/* Only permit special Q-register names for :q command */

    qnum = RdQnum(Fl_colon != 0);

    if (Skiptype != SKNONE)
	return(SKIPCONT);	/* continue skipping */

    if (Get1val(&t))		/* if there's a preceding value */
    {
	Eat_val();		/* discard values and flags now */
	retval = -1;		/* assume position is out of bounds */
	if ((TNUMB) t < TBsize(qnum))
	    retval = *(TBtext(qnum) + (TNUMB) t);
    }
    else			/* no current value */
    {
	retval = Getqval(qnum);	/* assume we want the numeric value */
	if (Fl_colon != 0)	/* if :Qq, return size of text portion */
	    retval = TBsize(qnum);
    }

    Set1val(retval);		/* use chosen value to set as current one */
    Eat_flags();		/* throw away current flags */
    return(XECCONT);
}
/* U command -- set Q register numeric value */

short
CMucmd()
{
    short qnum = RdQnum(FALSE);	/* normal values only */
    TNUMB val1;			/* first value */
    TNUMB val2;			/* second value */

    if (Skiptype != SKNONE)	/* quit now if skipping */
	return(SKIPCONT);
    
/* we do this in three steps:
 *	1. if there are two values, we set one and return the second.
 *	2. if there is just one value, we set it.
 *	3. if there are no values, an error results
*/

    if (Get2val(&val1,&val2))	/* get two values */
    {
	qvalue[qnum] = val2;	/* keep second value */
	Eat_val();		/* discard current values */
	Set1val(val1);		/* set first value as current one */
	return(XECCONT);	/* continue */
    }

    if (! Get1val(&val1))	/* error if no value */
	terrNUL(&Err_NAU);
    
    qvalue[qnum] = val1;	/* remember the value */
    Eat_val();
    Eat_flags();
    return(XECCONT);
}
/* % command -- increment Q register value */

short
CMper()
{
    short qnum = RdQnum(FALSE);	/* normal registers only */
    TNUMB incr;			/* increment amount */

    if (Skiptype == SKNONE)
    {
	Set1dfl(1);		/* default increment is 1 */
	(void) Get1val(&incr);	/* get the increment */

	qvalue[qnum] += incr;	/* bump value */
	Eat_val();		/* discard values, flags */
	Eat_flags();
	Set1val((TNUMB) qvalue[qnum]); /* set current Q register value */
    }
    return(CONTINUE);		/* continue executing/skipping */
}
/* [ command
**
** This command pushes a new value/text pair on the Q register stack.
*/

#define HDRSIZE	40			/* enough room for 2 integers, 2 :'s,
					** and a NUL
					*/

static int laststack;			/* position at which last stack frame
					** starts
					*/

short
CMlbrk()
{
    int qnum = RdQnum(FALSE);		/* get Q register number; regular ones
					** only
					*/
    char hdr[HDRSIZE];			/* stack frame header:  numeric value
					** and old stack frame
					*/
    int hdrlen;				/* number of characters in frame hdr */
    int curlen = TBsize(Q_stack);	/* current size of stack; (also, start
					** position of new frame)
					*/

    if (Skiptype != SKNONE)		/* continue skipping if skipping now */
	return(SKIPCONT);
    
    if (curlen <= 0)			/* keep "laststack" sane */
	laststack = 0;
    
    hdrlen = sprintf(hdr,"%d:%d:",Getqval(qnum),laststack);
					/* build header, get length, ignore
					** NUL in length
					*/
    
/* We could try to cheat by appending to the stack starting at the beginning
** of Q register qnum minus hdrlen, but that is potentially unsafe.  Therefore
** we append first the header, then the text of Q-qnum and back out on failure.
** We are also less elegant by always calling TBreord.
*/

    TBreorg();				/* get the maximum amount of space
					** possible
					*/
    Addst(Q_stack,hdr,hdrlen);		/* append header */
    
    if (! TBapp(Q_stack,TBsize(qnum),TBtext(qnum)))
					/* try to append text */
    {
	TBdel(Q_stack,curlen,hdrlen);	/* if failed, back out:  remove hdr */
	terrNUL(&Err_MEM);		/* produce error message */
    }

    laststack = curlen;			/* remember where current frame
					** begins
					*/
/* Note that numeric values are unaffected. */

    Eat_flags();			/* discard flag chars */
    return(XECCONT);			/* continue */
}
/* ] command
**
** Pop off of Q register stack
*/

short
CMrbrk()
{
    int qnum = RdQnum(FALSE);		/* get text block number, normal only */
    int oldstack;			/* start of previous stack frame */
    int numval;				/* retrieved numeric value */
    int textbeg;			/* start position of text portion
   					** of current frame
					*/
    char * s;				/* temporary string pointer */
    
    if (Skiptype != SKNONE)		/* continue skipping if now doing so */
	return(SKIPCONT);
    
/* do empty stack cases first */

    if (TBsize(Q_stack) <= 0)
    {
	if (Fl_colon == 0)		/* ] rather than :] */
	    terrNUL(&Err_PES);		/* attempt to pop empty stack */
	
	Eat_flags();			/* :] case:  discard flags */
	Set1val((TNUMB) -1);		/* set numeric value */
	return(XECCONT);
    }
/* We must maintain relative data about where things begin and end because
** of the TBreorg below.
*/

/* get numeric value and old stack frame position */

    s = TBtext(Q_stack) + laststack;	/* current start of stack frame */

    if (sscanf(s,"%d:%d:",&numval,&oldstack) != 2)
					/* should see 2 values */
	interr("Bad Q-stack frame (1)");
    
/* now we have to find the second :, since the text begins thereafter */

    s = strchr(s,':') + 1;		/* skip first : */
    s = strchr(s,':');			/* skip second */

#ifdef DEBUG
    if (s == (char *) 0)		/* check for not found */
	interr("Bad Q-stack frame (2)");
#endif

    s++;				/* bump pointer past second : */
    textbeg = s - TBtext(Q_stack);	/* start position for text */

/* now the fun begins.... */

    qvalue[qnum] = numval;		/* reset Q register numeric value */
    TBkill(qnum);			/*  discard old string content */

    if (! TBapp(qnum,TBsize(Q_stack)-textbeg,s))
					/* append tail-end of stack to qnum */
    {
	TBreorg();			/* failed.  Try reorganizing things */
	if (! TBapp(qnum,
		    TBsize(Q_stack)-textbeg,
		    TBtext(Q_stack)+textbeg
		    )
	    )
	    terrNUL(&Err_MEM);		/* failed again.  Memory overflow */
    }

/* Now delete this stack frame */

    TBdel(Q_stack,laststack,TBsize(Q_stack)-laststack);
    laststack = oldstack;		/* reset pos. of top stack frame */

    if (Fl_colon != 0)			/* for :], set a value */
	Set1val((TNUMB) 0);		/* mark success */

    Eat_flags();			/* discard flag chars */
    return(XECCONT);			/* continue */
}
