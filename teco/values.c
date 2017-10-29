static char SCCSID[] = "@(#) values.c:  5.1 2/25/83";
/* values.c
 *
 *	TECO value handling
 *
 *	This module performs the bizarre handling of numeric values for
 *	TECO.  It attempts to adhere to the User's Manual, but in at
 *	least one case it differs in favor of the PDP-11 version.
 *
 *	The idea here is that routines herein get called to set the
 *	two TECO values.  Since expressions get tangled up in all this,
 *	we also take care of arithmetic operators here too.
 *
 *	The philosophy of operation is something like this.  There are
 *	two potential values, which we'll call m and n for now.  For
 *	commands that take one value (4UA, e.g.), m is the one value.
 *	For commands like P that take two, m and n are the values:  m
 *	is the one before the comma.  There is the notion that a value
 *	does not exist, in which case a default gets used:  K means 1K,
 *	for example.  Routines in here deal with the defaults, too.
 *
 *	There is the notion of the "current value" which means m, before
 *	a comma, and n after a comma.  Standard TECO decrees that there
 *	cannot be a second comma, but TECO-11 does a funny sort of shift
 *	of operands, which is what we duplicate (see Nextval).
 *
 *	Expressions complicate things.  Generally there can be a pending
 *	operator.  Let's take the case of one operand.  When we see
 *	something like 1+..., the situation just after the + is:
 *
 *		1.  There is no current value (!)
 *		2.  There is a current operator, +
 *		3.  The mysteriously missing 1 is help off on the side as
 *		    the pending left operand.
 *
 *	We provide support for a value stack.  The external manifestations
 *	of the stack are the TECO ( and ) commands.  Unfortunately the
 *	behavior of pairs of values and operators was not well understood,
 *	so it may deviate from TECO-11.
 *
 *	Externally callable routines are:
 *
 *		Valinit -- effectively forget any current values, prepare
 *			for new ones (like effect of ESC in eating values)
 *		Set1val -- set a new value (current, either first or second)
 *		Set2val -- set both m and n
 *		Set1dfl -- set default for m
 *		Set012val -- set m and n for commands like K that take both
 *			positions and numbers of lines
 *		Setop -- set current operator
 *		Nextval -- move to "next" value (support for , )
 *		Get1val -- get m
 *		Get2val -- get m and n
 *		Pushv -- push current value set
 *		Popv -- pop value set from stack, combine with current
 */


/* Data local to the module but global within it */

#include "bool.h"			/* define booleans */
#include "errors.h"			/* define error structures */
#include "errmsg.h"			/* define error messages */
#include "find.h"			/* define search routines */
#include "mdconfig.h"			/* machine dependent config. */
#include "qname.h"			/* for text buffer name */
#include "xec.h"			/* define execution locations */



/* this must be defined here in addition to include file 'values.h' */

typedef int TNUMB;			/* TECO's number data type */


static TNUMB Value[3];			/* the two values, which we address
					 * as Value[1] and Value[2].  We
					 * waste Value[0]
					 */
static TNUMB Lop;			/* current pending left operand */
static TNUMB (* Operator)();		/* pending operator routine */

static BOOL Fl_value;			/* value flag:  TRUE if current
					 * value exists, FALSE if not
					 */
static short Curval;			/* current value number, 1 or 2 */

struct sf				/* stack frame */
{					/* essentially this consists of all
					 * of the above
					 */

    TNUMB value1;			/* first value */
    TNUMB value2;			/* second value */
    TNUMB lop;				/* pending left operand */
    TNUMB (* operator)();		/* pending operation */
    BOOL fl_value;			/* value present flag */
    short curval;			/* current value number */
};

/* define the stack */

static struct sf Stack[MAXSTACK];	/* stack array */


/* values available globally (mostly for zeroing and testing */

short Pstackp;				/* current paren stack offset */
short Stackdepth;			/* stack depth in current xec level */
/* nop -- no-operation operation expressions
 *
 * We define this here because it gets used a lot below
 */

static TNUMB
nop(l,r)
TNUMB l;
TNUMB r;
{
    return(r);				/* returns its right operand */
}
/* Valinit -- initialize value operations
 *
 * This routine initializes the world of values for later work.
 * Both values are undefined and the current value is 1.
 */

void
Valinit()
{
    Curval = 1;
    Fl_value = FALSE;			/* current value undefined */
    Lop = 0;				/* no left operand */
    Operator = nop;			/* current pending operator */
    Value[1] = Value[2] = 0;		/* for tidyness */
    return;
}
/* Set1val -- set single value as current
 *
 *	Set1val sets the current value and takes care of any pending
 *	operators.
 */

void
Set1val(m)
TNUMB m;				/* value to set */
{
    Value[Curval] = Operator(Lop,m);	/* perform pending operation, put
					 * result in current value
					 */
    Fl_value = TRUE;			/* declare presence of value */
    Operator = nop;			/* reset pending operation */
    Lop = 0;				/* reset left operand (tidy) */
    return;
}
/* Set2val -- set two values
 *
 * This routine sets both values and leaves the current value as 2.
 * It mimics the strange behavior of TECO-11 by setting the first
 * value from the first argument and by setting the second value with
 * the results of a pending operation and the second argument!
 */

void
Set2val(m,n)
TNUMB m;				/* first value */
TNUMB n;				/* second value */
{
    Value[1] = m;
    Curval = 2;
    Set1val(n);				/* this uses pending operator, if
					 * any
					 */
    return;
}
/* Set1dfl -- set single default
 *
 * This routine sets a first value default if there is no current value.
 * As with TECO-11, the default gets included with any pending operator.
 */

void
Set1dfl(m)
TNUMB m;				/* value to set */
{
    if (! Fl_value)			/* set default if we've seen nothing */
	Set1val(m);			/* note that this makes use of a
					 * pending operator, if any
					 */
    return;
}
/* Set012dfl -- set 0, 1, or 2 default values
 *
 * This routine takes care of commands that are both character position
 * and line-oriented.  If there are already two values, there is nothing
 * further to do.  Otherwise we set a single default and convert that to
 * two line positions.
 *
 * The peculiarities of TECO-11's handling of this function is duplicated.
 */

void
Set012dfl(m)
TNUMB m;				/* single default */
{
   Set1dfl(m);				/* take pending operation into
					 * account.  Note that pending
					 * operation becomes nop
					 */
    if (Curval == 1)			/* if we only have one value so far */
    {
	int pos = Findlt(Q_text,Dot,(int) Value[1]);
					/* find appropriate line terminator
					 * starting at current pos
					 */

	if (pos >= Dot)			/* order positions correctly */
	    Set2val(Dot,pos);		/* set the two values */
	else
	    Set2val(pos,Dot);
    }
    return;
}
/* Setop -- set operation
 *
 * This routine sets up a pending operation which gets completed the
 * next time a value is provided.  All such operators are assumed to
 * be binary in-fix.
 *
 * One odd case that gets handled here is consecutive operators, as in
 * 3+*4.  The result in TECO-11 is 12 because the missing operand is
 * treated as 0.
 */

void
Setop(op)
TNUMB (* op)();				/* pointer to function */
{
    Lop = Operator(Lop,Value[Curval]);	/* save left operator, make use of
					 * any pending operation.  This has
					 * the effect, generally, of using
					 * 0 as the right operand except
					 * when the pending operation is nop.
					 */
    Value[Curval] = 0;			
    Fl_value = FALSE;			/* claim no current value */
    Operator = op;			/* set pending operator routine */
    return;
}
/* Nextval -- skip to next value
 *
 * This routine is essentially support for the , operator.
 *
 * It diverges from the documentation, which prohibits more than 1 , in
 * succession, but agrees with TECO-11.
 */

void
Nextval()
{
    if (! Fl_value)
	terrNUL(&Err_NAC);		/* error if no preceding arg */

/* if we get here there must be a current operand (and, incidentally, no
 * pending operation).
 */

/* Keep this test for machines where TNUMBs are >32767.  On the PDP-11,
** the startup code uses 32768 as a value to ',' , which looks negative.
** The problem is with the m,nET command, where one of the valid ET
** values is 32768.
*/

    if (sizeof(TNUMB) > 2 && Value[Curval] < 0)	/* negative not allowed */
	terrNUL(&Err_NCA);

    if (Curval == 2)
	Value[1] = Value[2];		/* if we have 2 values, shift! */
    
    Curval = 2;				/* now we definitely have 2 values */
    Operator = nop;			/* reset for tidyness */
    Fl_value = FALSE;			/* !! to simulate TECO-11's behavior */
    return;
}
/* Get1val -- get single value
 *
 * This routine returns a single value and a flag signifying whether there
 * is a value defined.
 *
 * For TECO-11 compatibility we return the current value, not the first
 * one!  Furthermore, if there is a pending operation, the current value
 * will be 0, which is what we return.
 */

BOOL					/* whether value defined or not */
Get1val(pm)
TNUMB * pm;				/* point to place to store it */
{
    *pm = Value[Curval];
    return(Fl_value);
}
/* Get2val -- get both values
 *
 * This routine returns both numeric values and a flag, as above.
 * Callers must insure that both values are defined.
 *
 * For TECO-11 compatibility we return the current values, ignoring
 * pending operations.
 */

BOOL					/* second value is defined */
Get2val(pm,pn)
TNUMB * pm;				/* pointer to place to store first */
TNUMB * pn;				/* ... and second */
{

    if (Curval != 2)
	return(FALSE);			/* there aren't two values */

    *pm = Value[1];
    *pn = Value[2];
    return(Fl_value);
}
/* Pushv -- push data onto value stack
 *
 * This routine records the current state of the value world on the
 * stack.
 */

void
Pushv()
{
    struct sf * sp;			/* stack frame pointer */

    if (Pstackp >= MAXSTACK)		/* check for too deep */
	terrNUL(&Err_PDO);		/* push down overflow */
    
    sp = &Stack[Pstackp++];		/* address of current sf */
    Stackdepth++;			/* bump current depth */

    sp->value1 = Value[1];		/* transcribe everything */
    sp->value2 = Value[2];
    sp->lop = Lop;
    sp->operator = Operator;
    sp->fl_value = Fl_value;
    sp->curval = Curval;

    Valinit();				/* now reset the value mechanism */
    return;
}
/* Popv -- back up one stack level, combine with current values
 *
 * This routine unwinds from Pushv and attempts to combine the current
 * values with those in the stack frame.
 */

void
Popv()
{
    TNUMB v1=Value[1];			/* remembered current value */
    TNUMB v2=Value[2];
    BOOL oneval=(Curval==1);		/* how many values there are */
    struct sf * sp;			/* stack pointer */

    if (Stackdepth <= 0)		/* check for too many ) */
	terrNUL(&Err_MLP);		/* missing ( */
    if (! Fl_value)			/* if no value... */
	terrNUL(&Err_NAP);		/* no arg before ) */
    
/* unwind stack:  remember that the stack frame data is lexically to the
 * left of the current stuff.  We must get the left and right operands in
 * the proper order.
 */

    sp = &Stack[--Pstackp];		/* point at stack frame */
    Stackdepth--;			/* decr. stack depth */
    Value[1] = sp->value1;
    Value[2] = sp->value2;
    Lop = sp->lop;
    Operator = sp->operator;
    Fl_value = sp->fl_value;
    Curval = sp->curval;

    if(oneval)				/* now combine old and new values */
	Set1val(v1);			/* just 1 new value */
    else
	Set2val(v1,v2);			/* set both */
    
    return;
}
