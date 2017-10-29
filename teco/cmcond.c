static char SCCSID[] = "@(#) cmcond.c:  4.1 12/12/82";
/* cmcond.c
 *
 *	TECO conditional command processor
 *
 *	David Kristol, March, 1982
 *
 *	This module contains all of the TECO conditional commands.
 *	These include the multiple versions of test, and else and fi
 *	operators:
 *	" | '
 */

#include "bool.h"
#include "ctype.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "skiptype.h"
#include "values.h"
#include "xec.h"

int Condnest;			/* conditionals nesting level:  use this
				** to keep track of all conditionals we
				** enter after the first.  We're only done
				** with conditionals when the level gets
				** back to zero
				*/
/* " (conditional test) command */

short
CMdquot()
{
    BOOL condtest();
    int condchar;			/* character following " */

    if ((condchar = gCMch()) < 0)	/* get test character */
	Unterm();			/* error if failed */

    if (Skiptype != SKNONE)		/* if already skipping ... */
    {
	Condnest++;			/* bump level of conditionals */
	return(SKIPCONT);		/* and continue */
    }

    if (! condtest(condchar))
	if (Skip(SKCOND) == SKIPRANOFF) /* skip if conditional fails */
	    Unterm();    		/* unterminated command */

    return(XECCONT);	        	/* otherwise, continue executing */
}
/* condtest -- do conditional test for " command */

/* macro for below */

#define ISCHAR(c)	(0 <= c && c <= 0177)
BOOL
condtest(condchar)
int condchar;				/* character following " */
{
    TNUMB t;				/* current value */

    if (!Get1val(&t))
	terrNUL(&Err_NAQ);		/* make sure there's a value */
    
/* discard flags and values */

    Eat_flags();
    Eat_val();

/* branch on type of test */

    switch (condchar)
    {
    case 'c':				/* symbol constituent */
    case 'C':
	return( ISCHAR(t) && ISSYM(t) );

    case 'r':				/* letter or digit */
    case 'R':
	return( ISCHAR(t) && (ISLET(t) || ISDIG(t)) );

    case 'a':				/* upper or lower case letter */
    case 'A':
	return( ISCHAR(t) && ISLET(t) );
    
    case 'd':				/* digit */
    case 'D':
	return( ISCHAR(t) && ISDIG(t) );
    
    case 'e':				/* equal 0 */
    case 'E':
    case 'f':				/* false */
    case 'F':
    case 'u':				/* unsuccessful */
    case 'U':
    case '=':				/* = 0 */
	return( t == 0 );
    

    case 'g':				/* > 0 */
    case 'G':
    case '>':
	return ( t > 0 );
    
    case 'l':				/* < 0 */
    case 'L':
    case 's':				/* successful */
    case 'S':
    case 't':				/* true */
    case 'T':
    case '<':
	return ( t < 0 );
    
    case 'n':				/* not = 0 */
    case 'N':
	return ( t != 0 );
    
    case 'v':				/* lower case letter */
    case 'V':
	return( ISCHAR(t) && ISLET(t) && ISLC(t) );

    case 'w':				/* upper case letter */
    case 'W':
	return( ISCHAR(t) && ISLET(t) && ISUC(t) );
    
    default:				/* anything else */
	terrCHR(&Err_IQC,condchar);	/* illegal " char */
    }
/*NOTREACHED*/
}
/* ' command:  end of conditional */

short
CMapos()
{
/* depends on skipping type */

    switch (Skiptype)
    {
    case SKNONE:			/* not skipping at all */
	Eat_flags();			/* eat flags, but not values */
	return(XECCONT);		/* continue execution */
    
    case SKCOND:			/* looking for | or ' */
    case SKFI:				/* looking only for ' */
	if (Condnest == 0)		/* reach end of conditional at
					** desired level?
					*/
	    return(SKIPDONE);		/* yes.  Done skipping */
    	/* otherwise, fall through */

    default:				/* for others, continue */
	if (--Condnest < 0)		/* drop a level of conditional,
					** but stay non-negative
					*/
	    Condnest = 0;
	return(SKIPCONT);
    }
/*NOTREACHED*/
}
/* | command:  else clause */

short
CMbar()
{
/* dependent on skipping condition */

    switch (Skiptype)
    {
    case SKNONE:			/* not skipping */
	if (Skip(SKFI) == SKIPDONE)	/* skip to ' */
	    return(XECCONT);		/* continue on success */
	Unterm();			/* otherwise, unterminated command */
    
    case SKCOND:			/* looking for part of conditional */
	if (Condnest == 0)		/* if at top-level conditional, done */
	    return(SKIPDONE);		/* found it ! */
    
    default:				/* for anything else, continue */
	return(SKIPCONT);
    }
/*NOTREACHED*/
}
