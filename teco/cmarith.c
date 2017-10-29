static char SCCSID[] = "@(#) cmarith.c:  4.1 12/12/82";
/* cmarith.c
 *
 *	TECO arithmetic package
 *
 *	David Kristol, February, 1982
 *
 *	This module contains TECO's arithmetic operators and its
 *	number readers.  These commands are supported:
 *	+ - * / & # ^_ ( ) \ and digits
 */



#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include "bool.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "values.h"
#include "xec.h"



/* Number reader */

static int				/* return digit or -1 */
isdigit(c)				/* test whether char is digit in
					 * current radix
					 */
int c;					/* character to check */
{
    switch (c)
    {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
	return(Radix == 16 ? c-'a'+10 : -1);
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	return(Radix == 16 ? c-'A'+10 : -1);
    case '8':
    case '9':
	if (Radix == 8)
	    return(-1);
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
	return(c-'0');
    default:
	return(-1);
    }
}
/* handle digits */
short
CMdigit()
{
    TNUMB t = 0;			/* current number value */
    int c = Cmdchar;			/* current character (first one
					 * is already read
					 */
    short digit;			/* current digit */

    if (Skiptype != SKNONE)		/* if skipping, forget it */
	return(SKIPCONT);

/* Exiting if not skipping is an insurmountable bug when the radix is
 * 16.  The problem with hex numbers is that they can look like TECO
 * commands.  Since when we are skipping we cannot be conscious of the
 * current radix (it could change several times, after all), we must
 * hope for the best, namely that any hex constants have no letters
 * in them, or at least that the letters are harmless combinations
 * that will get skipped as well.
 * E and F are particular problems!
 */

/* as we entered with a digit, we cannot be looking at a hex letter.
 * isdigit will fail if the radix is octal and the digit is 8 or 9
 */

    if ((t = isdigit(c)) < 0)
	terrNUL(&Err_ILN);		/* produce error */

    while ((digit = isdigit(pCMch())) >= 0)
    {
	(void) gCMch();			/* now safe to read the char */
	t = Radix * t + digit;		/* add in new value */
    }
    Set1val(t);				/* set this value */
    return(XECCONT);
}
/* Simple binary operators */
/* We give the action routine, then the command routine.  The action
 * routine is supplied to the value mechanism and gets called when a
 * right hand operand is supplied
 */

TNUMB
plus(l,r)
TNUMB l,r;
{
    return(l+r);
}

short
CMplus()				/* addition */
{
    if (Skiptype == SKNONE)
	Setop(plus);
    return(CONTINUE);
}

TNUMB
minus(l,r)
TNUMB l,r;
{
    return(l-r);
}

short
CMminus()				/* subtraction */
{
    if (Skiptype == SKNONE)
	Setop(minus);
    return(CONTINUE);
}
/* multiplicative operators */

TNUMB
times(l,r)
TNUMB l,r;
{
    return(l*r);
}

short
CMtimes()				/* multiplication */
{
    if (Skiptype == SKNONE)
	Setop(times);
    return(CONTINUE);
}

TNUMB
div(l,r)
TNUMB l,r;
{
    return(r == 0 ? 0 : l/r);
}

short
CMslash()				/* division */
{
    if (Skiptype == SKNONE)
	Setop(div);
    return(CONTINUE);
}
/* boolean operators */

TNUMB
and(l,r)
TNUMB l,r;
{
    return(l & r);
}

short
CMamp()					/* bit-wise and */
{
    if (Skiptype == SKNONE)
	Setop(and);
    return(CONTINUE);
}

TNUMB
or(l,r)
{
    return(l | r);
}

short
CMpound()				/* bit-wise or */
{
    if (Skiptype == SKNONE)
	Setop(or);
    return(CONTINUE);
}
/* This last operator is a bit different from the others.
 * ^_ (logical complement) eats a value and sets a new one.
 */

short
CMctund()				/* 1's complement */
{
    TNUMB t;				/* returned value */

    if (Skiptype != SKNONE)
	return(SKIPCONT);

    if (!Get1val(&t))
	terrNUL(&Err_NAB);		/* no argument before ^_ */
    
    Eat_val();				/* clear out current values */
    Set1val(~t);			/* set new value as complement */
    return(XECCONT);
}
/* CMlpar -- process left parenthesis */

short
CMlpar()
{
    if (Skiptype == SKNONE)
	Pushv();			/* push values on stack */
    return(SKIPCONT);
}


/* CMrpar -- process right parenthesis */

short
CMrpar()
{
    if (Skiptype == SKNONE)
	Popv();				/* pop values from stack */
    return(SKIPCONT);
}
/* \ command */

/* The \ command has two variants, depending on whether there is a value
** available or not.  With a value, the value is entered into the text
** buffer at the current position in the current radix.  Without a value,
** \ scans characters in the text buffer and forms a value, according to
** the current radix.
*/

short
CMbksl()
{
    TNUMB val;				/* current value */
    int intval;				/* integer value */
    char * fmt;				/* format to use for conversion */
    char buf[30];			/* temporary buffer for converting
					** value to characters
					*/
    int len;				/* length of insert or number string */
    int sprintf();

    if (Skiptype != SKNONE)		/* exit now if skipping */
	return(SKIPCONT);
    
    if (Get1val(&val))			/* determine which case */
    {					/* convert value to ASCII */
	fmt = "%d";			/* default output format */
	if (Radix == 8)			/* octal */
	    fmt = "%o";
	else if (Radix == 16)		/* hex */
	    fmt = "%x";
	
	intval = val;			/* convert value to int */
	if (!TBins(	Q_text,
			Dot,
			(len = sprintf(buf,fmt,intval)),
			buf
		    )
	    )
	    terrNUL(&Err_MEM);		/* memory overflow if insert fails */
	
	Dot += (Inslen = len);		/* bump '.', set insert length */
	Eat_flags();			/* discard flags and values */
	Eat_val();
    }
/* convert characters in buffer to value */
    else
    {
	char * buffirst = TBtext(Q_text); /* pointer to start of buffer */
	int bufsize = TBsize(Q_text);	/* size of buffer (to bound \) */
	int digit;			/* current digit */
	int sign = 1;			/* current sign is + */
	char c;				/* temporary character */

	val = 0;			/* current value */

	/* take care of leading sign */
	if (Dot < bufsize)
	{
	    if ((c = *(buffirst+Dot)) == '+')
		Dot++;			/* just skip + */
	    else if (c == '-')
	    {
		sign = -1;		/* make minus sign */
		Dot++;			/* and skip sign char */
	    }
	}

	/* now scan digits in buffer */

	while (Dot < bufsize)		/* confine to buffer */
	{
	    if ((digit = isdigit(*(buffirst+Dot))) < 0)
		break;			/* no more digits */

	    val = Radix * val + digit;	/* add in new digit */
	    Dot++;			/* scan over current char */
	}
	Set1val((TNUMB) val * sign);	/* set the signed value */
    }
    return(XECCONT);
}
