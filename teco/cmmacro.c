static char SCCSID[] = "@(#) cmmacro.c:  5.1 2/22/83";
/* cmmacro.c
 *
 *	TECO macro and iteration commands
 *
 *	David Kristol, March, 1982
 *
 *	This module contains these commands in TECO:
 *	Iteration:
 *	    < > ;
 *	Macro:
 *	    M
 *	Control:
 *	    $ ESC F> F< F' F| ^C
 */

#include <setjmp.h>
#include "bool.h"
#include "chars.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "qname.h"
#include "skiptype.h"
#include "tflags.h"
#include "values.h"
#include "xec.h"
#include "xectype.h"
/* module-global data */

int Iterpos;				/* starting position of current
					 * iteration
					 */
int Iternest;				/* nesting level for iterations being
					** skipped
					*/
static int Itercount;			/* iteration count for current
					 * iteration or -1 for "infinite"
					 */
/* < command:  start iteration */

short
CMlt()
{
    TNUMB itercount;			/* temporary iteration count */
    BOOL infinite = ! Get1val(&itercount); /* TRUE if loop has no limit */

    void pushiter();

    if (Skiptype != SKNONE)		/* if skipping, skip entire
					 * iteration
					 */
    {
	Iternest++;			/* down one level in iterations */
	return(SKIPCONT);		/* continue skipping */
    }
/* since we're not skipping, process the iteration */

    Eat_val();				/* discard values and flags */
    Eat_flags();

    if (! infinite && itercount <= 0)	/* zero-trip iteration */
    {
	/* skip nesting levels zeroed by Skip */
	if(Skip(SKITER) == SKIPDONE)	/* skip over */
	    return(XECCONT);		/* and continue */
	else
	    terrNUL(&Err_MRA);		/* else, error:  missing > */
    }

/* now we're really going to do it */

    if (infinite)
	itercount = -1;			/* change local iteration count
					 * to mean "infinite"
					 */
    pushiter(itercount);		/* make us start interpreting an
					 * iteration now
					 */
    return(XECCONT);			/* and continue */
}
/* > command:  end of iteration */

short
CMgt()
{
    void popiter();
    void interr();

    switch(Skiptype)			/* skipping dependent */
    {
    case SKNONE:			/* not skipping */
	if (Xectype != XECITER)		/* not in iteration */
	    terrNUL(&Err_BNI);		/* produce error */
    
    /* we are in an iteration */

	if (Itercount < 0		/* infinite */
	    || --Itercount > 0)		/* or count still good */
	    Cmddot = Iterpos;		/* reset command position for
					 * another go-round
					 */
	else
	    popiter();			/* otherwise terminate loop */
	Eat_val();			/* discard current values and flags */
	Eat_flags();
	return(XECCONT);		/* and continue execution */

    case SKITER:			/* looking for end of iteration */
	if (Iternest == 0)		/* if at top level iteration */
	    return(SKIPDONE);		/* all done */
	
	--Iternest;			/* otherwise, drop one level of
					** skipping
					*/
	return(SKIPCONT);		/* and continue skipping */

    case SKGOTO:			/* goto */
    case SKFI:				/* looking for ' */
    case SKCOND:			/* looking for ' or | */
    /* if we cross over > at top level, exit the corresponding iteration */
	if (--Iternest < 0)		/* at top level? */
	{
	    Iternest = 0;		/* yes.  keep non-negative level */
	    if (Xectype == XECITER)	/* if we were in iteration, get out
					** of it
					*/
		popiter();
	}
	return(SKIPCONT);		/* for others, continue skipping */

    default:				/* everything else */
	interr("Skip type not handled by >");
    }
/*NOTREACHED*/
}
/* ; : branch out of iteration */

short
CMsemi()
{
    TNUMB t;				/* current value */
    void popiter();

    if (Skiptype != SKNONE)		/* ignore if skipping */
	return(SKIPCONT);

    if (Xectype != XECITER)		/* check in iteration */
	terrNUL(&Err_SNI);
    
    if (! Get1val(&t))			/* if no value */
	terrNUL(&Err_NAS);		/* missing arg */
    
    if (Fl_colon != 0)			/* colon inverts sense */
	t = -t-1;			/* mach. indep. 1's comp. */
    
    Eat_flags();
    Eat_val();
    if (t < 0)				/* should we exit loop? */
	return(XECCONT);		/* no, continue executing */
    
    if(Skip(SKITER) != SKIPDONE)	/* yes.  skip over rest of loop */
	Unterm();			/* if failed, unterminated command */

    popiter();				/* get out of loop */
    return(XECCONT);
}
/* Iteration stack management routines */

/* pushiter -- save current execution state, start iteration
 *
 * This routine remembers what needs to be remembered when starting a new
 * loop.  The complementing function is popiter.
 */

static void
pushiter(i)
int i;
{
    void Pushx();

    Pushx(Iterpos);			/* save current iteration position */
    Pushx(Itercount);			/* and count */
    Pushx(Xectype);			/* and execution type */

    Iterpos = Cmddot;			/* remember current position as place
					 * to loop to
					 */
    Itercount = i;			/* set current count */
    Xectype = XECITER;			/* current execution type */
    return;
}

/* popiter -- get out of current iteration
 *
 * This routine unwinds from the current execution level which is presumed
 * to be an iteration.  It complements pushiter.
 */

static void
popiter()
{
    void interr();
#ifdef DEBUG
    if (Xectype != XECITER)
	interr("not in iteration in popiter");
#endif

    Xectype = Popx();			/* get old execution type */
    Itercount = Popx();			/* previous iteration count */
    Iterpos = Popx();			/* previous iteration position */
    return;
}
/* M command
**
** Execute macro.
*/

short
CMmcmd()
{
    short RdQnum();
    void pushmacro();

    short tb = RdQnum(FALSE);		/* read Q register name, regular
					** names only
					*/

    if (Skiptype == SKNONE)		/* only enter the macro if
					** not skipping
					*/
	pushmacro(tb);			/* enter new macro */
    return(CONTINUE);
}
/* pushmacro -- enter new macro
**
** This routine records the necessary information on the control stack
** to enter a macro.  The stack frame contains the current paren stack
** depth, the current command position (text block and position) and
** old execute state.  Then new values for these items are set.
*/

static void
pushmacro(tb)
short tb;				/* text block to execute */
{
    void Pushx();

    Pushx(Stackdepth);
    Pushx(Cmdtb);
    Pushx(Cmddot);
    Pushx(Xectype);

    Cmdtb = tb;				/* set new command text block */
    Cmddot = 0;				/* start at beginning */
    Stackdepth = 0;			/* reset paren stack depth:  parens
					** must balance in the new macro
					*/
    Xectype = XECMACRO;			/* now inside a macro */
    return;
}
/* popmacro -- exit from macro
**
** This routine unwinds from the above.
*/

static void
popmacro()
{
    int Popx();

/* sanity check:  make sure we're inside a macro */

#ifdef DEBUG
    void interr();
    if (Xectype != XECMACRO)
	interr("popmacro not in macro");
#endif

    if (Stackdepth != 0)		/* parens must have been balanced
					** in the macro we're exiting
					*/
	terrNUL(&Err_MRP);		/* missing right paren */
    
    Xectype = Popx();
    Cmddot = Popx();
    Cmdtb = Popx();
    Stackdepth = Popx();
    return;				/* and continue in previous text
					** block
					*/
}
/* ESC and $ command
**
** If single escape, eat current values and flags.  If followed by a
** second ESC, don't eat values; exit from current macro instead.
*/

short
CMesc()
{
    short Exitmacro();

    if (Skiptype != SKNONE)		/* continue skipping if now skipping */
	return(SKIPCONT);

    if (pCMch() == ESC)			/* peek at next command character */
    {
	(void) gCMch();			/* saw ESCape:  eat character */
	return(Exitmacro(FALSE));	/* exit, don't eat values */
					/* (Semantics of $$ are to preserve
					** values and return them.)
					*/
    }

/* normal case:  single ESC.  Eat values, continue executing */

    Eat_val();
    Eat_flags();
    return(XECCONT);
}
/* Exitmacro -- exit macro
**
** This routine does a forced macro exit.  If we are currently within any
** iterations, we pop out of them first.  Our return indication says
** whether we are at top level.
*/

short					/* XECCONT to continue, XECDONE
					** if exiting top level
					*/
Exitmacro(flag)
BOOL flag;				/* TRUE to kill current values */
{
    if (flag)
    {
	Eat_val();
	Eat_flags();
    }

    while (Xectype == XECITER)		/* exit iterations first, if any */
	popiter();
    
    if (Xectype == XECINPUT)		/* if now at top level... */
	return(XECDONE);		/* signal we are now done */
/* Note that we don't fuss with balanced parens when returning from top
** level.  The principle reason (using the usual mushy TECO semantics)
** is this:  since our stack discipline is so weak, we must be sure, when
** exiting a macro, that the paren stack is where it was when we entered.
** Otherwise things would get horribly messed up.  When returning to top
** level, however, there isn't anything further that can get messed up,
** so we simply don't worry about mismatched parens here.
*/
    
    popmacro();				/* otherwise, exit current macro */
    return(XECCONT);			/* and continue in previous one */
}
/* F control commands:  F>  F<  F'  F|  */

short
Fctrl(c)
int c;					/* particular control command char */
{
    if (Skiptype != SKNONE)		/* ignore all of these if skipping */
	return(SKIPCONT);
    
    Eat_flags();			/* discard flags and values for all */
    Eat_val();

    switch(c)				/* dispatch on specific command */
    {
    case '>':				/* F>:  depends on current execution
					** state
					*/
	switch(Xectype)
	{
	case XECITER:			/* currently in iteration */
	    if (Skip(SKITER) == SKIPDONE) /* if successful to skip to > */
		return(CMgt());		/* do > and return result */
	    else
		Unterm();		/* otherwise, unterminated command */
	
	case XECMACRO:			/* in macro */
	case XECINPUT:			/* top level */
	    return(Exitmacro(TRUE));	/* return result of exiting macro
					** level; TRUE is redundant
					*/
#ifdef DEBUG
	default:			/* should be superfluous */
	    interr("missed state in Fctrl F>");
#endif
	}
/* F< */

    case '<':				/* F< */
	Cmddot = (Xectype == XECITER ? Iterpos : 0);
					/* loop to beginning of iteration or
					** macro
					*/
	return(XECCONT);		/* continue execution */
    

    case '\'':				/* F' */
    /* search for matching ' in conditional */

	if (Skip(SKFI) == SKIPDONE)	/* if successful search for ' */
	    return(XECCONT);		/* continue executing */
	terrNUL(&Err_MAP);		/* else, missing ' */
    

    case '|':
    /* search for matching | or ' in conditional */

	if (Skip(SKCOND) == SKIPDONE)	/* if success, continue executing */
	    return(XECCONT);
	terrNUL(&Err_MAP);

#ifdef DEBUG
    default:				/* pick up stragglers */
	interr("bad F char in Fctrl");
#endif
    }
/*NOTREACHED*/
}
/* ^C command:
**
** Exit to top level or exit TECO
*/

extern TNUMB ET;			/* TECO ET flag */
extern jmp_buf Reset;			/* longjmp target */
extern void longjmp();

short
CMctccmd()
{
    if (Skiptype != SKNONE)		/* if skipping, continue to do so */
	return(SKIPCONT);

    if (
	(pCMch() == CTRLC) ||		/* 2 successive ^C's */
	(Cmdtb == Q_cmd) ||		/* executing from command buffer
					** (i.e., not in a macro)
					*/
	((ET & ET_mung) != 0)		/* ^C in MUNG */
	)
    {
	TNUMB exitcode;			/* TECO exit code */
	extern void Tecexit();

	Set1dfl((TNUMB) 0);		/* set default exit code */
	(void) Get1val(&exitcode);
	Tecexit((int) exitcode);	/* pass out exit code and exit */
    }
    else
	longjmp(Reset,CTRLCEXIT);	/* otherwise return to top level */
/*NOTREACHED*/
}
