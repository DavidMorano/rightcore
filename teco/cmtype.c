static char SCCSID[] = "@(#) cmtype.c:  5.2 4/8/83";
/* cmtype.c
 *
 *	TECO type-out commands
 *
 *	David Kristol, February, 1982
 *
 *	This module contains TECO's type-out commands:
 *
 *	T ^A = V ^T ^L (FF)
 *
 */


#include <stdio.h>

#include "bool.h"
#include "chars.h"
#include "cmdio.h"
#include "cmutil.h"
#include "ctype.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "find.h"
#include "qname.h"
#include "skiptype.h"
#include "tflags.h"
#include "tty.h"
#include "values.h"
#include "xec.h"



extern TNUMB ET;			/* ET flag */
extern TNUMB EU;			/* EU flag */


/* T command */

short
CMtcmd()
{
    int first;				/* first position to print */
    int len;				/* amount to print */

    if (Skiptype != SKNONE)
	return(SKIPCONT);

    Confine(1,&first,&len);		/* default = 1, get first, len */

    wTBst(Q_text,first,len,Cmdout);	/* write these positions */

    Eat_val();				/* gobble values and flags */
    Eat_flags();
    return(XECCONT);
}
/* ^A command */

short
CMcta()
{
    int start;				/* starting point of string */
    int len;				/* length of string */

    Finddlm(CTRLA,&start,&len);		/* look for ^A, get bounds */

    if (Skiptype != SKNONE)
	return(SKIPCONT);
    
    wTBst(Cmdtb,start,len,Cmdout);	/* write the intervening chars */
    Eat_val();
    Eat_flags();
    return(XECCONT);
}
/* = command -- type out number in decimal/octal/hex */

/* these are formats used for the = command */

static char * fmt[] = {
"%d\r\n",	"%d",	"%o\r\n",	"%o",	"%X\r\n",	"%X" };
/*  0		  1	   2		 3	   4		 5
   n=		n:=	  n==		n:==	  n===		n:=== */

short
CMeq()
{
    short fnum = 0;			/* initial format number to use */
    TNUMB t;				/* value to print out */
    int len;				/* length of output string */
    char outstring[40];			/* output character string:  big
					** enough for anything reasonable
					*/
    char * s;				/* pointer to digit string */

    if (Skiptype != SKNONE)
	return(SKIPCONT);		/* continue skipping if skipping */
    
    if (! Get1val(&t))			/* error if no value available */
	terrNUL(&Err_NAE);

/* we must poke through trailing characters to decide which format to use */

    if (pCMch() == '=')			/* is command == ? */
    {
	(void) gCMch();			/* yes.  discard character */
	fnum += 2;			/* and bump format */
	if (pCMch() == '=')		/* check for yet another = */
	{
	    (void) gCMch();		/* discard it */
	    fnum += 2;			/* and bump format again */
	}
    }

    if (Fl_colon != 0)			/* use alternate format if : */
	fnum++;

/* we have chosen a format.  Now print value */

    len = sprintf(outstring,fmt[fnum],t);

    for (s = outstring; len > 0; len--)
	wCch(*s++,Cmdout);		/* output digit string */

    Eat_flags();			/* discard flags and value */
    Eat_val();
    return(XECCONT);			/* continue executing */
}
/* Dovsub -- do guts of V command
**
** This routine is used in several places to output a set of lines around
** '.'.
*/

void
Dovsub(n,c)
int n;					/* number of lines in window */
int c;					/* character to use to mark '.' */
{
    void wCch();

/* nV is the same as -(n-1)TnT.  Find the lower and upper bounds for
** output.
*/

    int first = Findlt(Q_text,Dot,1-n);	/* lower bound in text buffer */
    int last = Findlt(Q_text,Dot,n);	/* upper bound in text buffer */

    wTBst(Q_text,first,Dot-first,Cmdout); /* write from lower bound to '.' */

    if (c != NUL)			/* write non-null characters only */
	wCch(c,Cmdout);

    wTBst(Q_text,Dot,last-Dot,Cmdout);	/* write from '.' to upper bound */
    return;
}
/* V command */
/* Beware of the meaning of m,nV which is somewhat irregular! */

short
CMvcmd()
{
    TNUMB val;				/* preceding value */
    TNUMB m;				/* args for m,nV */
    TNUMB n;

    if (Skiptype != SKNONE)		/* don't do V if skipping */
	return(SKIPCONT);

    Set1dfl(1);				/* set default value of 1 */
    if (Get2val(&m,&n))			/* if two values available, m,nV */
    {
	int first = Findlt(Q_text,Dot,(int) -m); /* find start position */
	int last = Findlt(Q_text,Dot,(int) n);   /* find end position */
	int len = last - first;		         /* apparent length of write */

	if (len < 0)			/* reverse order */
	{
	    len = -len;
	    first = last;		/* start with previous last */
	}
	wTBst(Q_text,first,len,Cmdout);	/* write the text */
    }
    else
    {
	(void) Get1val(&val);		/* get current value */

	Dovsub(val,NUL);		/* call subr. to do the work */
    }
    Eat_val();				/* discard flags and values */
    Eat_flags();
    return(XECCONT);			/* and continue */
}
/* Dovflag -- handle certain flags that cause type-out
**
** This routine manages the details of such flags as ES which cause
** parts of the text buffer to be printed.  The behavior depends on
** the argument n, and it goes like this:
**
**	-1		do V
**	0		do nothing
**	1 - 31		do V command with LF at '.'
**	32 - 126	do V command with ASCII(n) displayed at '.'
**	127 - 255	do V command with NUL displayed at '.'
**	m*256 + c	do mV command with ASCII(c) displayed at '.'
*/

void
Dovflag(n)
int n;					/* flag value to work with */
{
    int nlines = 0;			/* number of lines to print */
    char c = NUL;			/* character to display at '.' */

    /* In the absence of other choices, one line gets printed with no
    ** special separator.
    */

    if (n == 0)				/* do-nothing case */
	return;
    
    if (n > 0)
    {
	c = n & 0377;			/* extract character */
	nlines = (n >> 8) & 0177;	/* get number of lines */
    }

    if ( 1 <= c && c <= 31 )		/* force LF */
	c = LF;
    else if ( c >= 127 )		/* force NUL */
	c = NUL;

    if (nlines == 0)
	nlines = 1;			/* force at least one line */

    Dovsub(nlines,c);			/* now print the lines */
    return;
}
/* ^T command -- output character/input character
**
** This command is another of those that behave in two ways, depending
** on whether there is a value available.  With a value, we output the
** single character whose ASCII code is the value.  Without a value,
** we read a character from the terminal.  If the ET_rnoe bit is off,
** we echo the character we read.
**
** One special case for character reading:  if the ET flag is set so
** we do not wait on a read, rIch will return -1, which is the defined
** value for a failing ^T.  We don't echo anything for this value.
*/

short
CMcttcmd()
{
    TNUMB c;				/* character output/input */
    void wCch();
    int rIch();
    static BOOL lastcr = FALSE;		/* TRUE if last char read is CR */
    void outctt();			/* local routine to output ^T char */

    if (Skiptype != SKNONE)		/* just exit if skipping */
	return(SKIPCONT);

    if (Get1val(&c))			/* have a value? */
    {					/* yes:  output char */
	outctt( c & 0177 );
	Eat_val();			/* discard values and flags */
	Eat_flags();
	return(XECCONT);
    }
    /* Handle read-a-character case */

    c = (TNUMB) rIch(TRUE);		/* read a character (could return
					** -1 if set up for non-blocking
					** reads); convert CR to CR/LF
					*/

    if ( c >= 0 )			/* if got a character */
    {
	if (    ISLC( c )		/* Translate case of alphabetics
					** if we can't read lower case
					*/
	    &&  ISLET( c )
	    &&  (ET & ET_rlc) == 0
	    )
	    c = UPPER( c );		/* convert to upper case */

	if ((ET & ET_rnoe) == 0)	/* if we echo stuff */
	{
	    if (c == CR)
	    {
		Crlf(Cmdout);		/* output CRLF */
		lastcr = TRUE;		/* remember we saw CR */
	    }
	    else if ((! lastcr) || c != LF) /* not a LF after a CR */
	    {
		outctt((char) c);	/* output character */
		lastcr = FALSE;		/* not following CR */
	    }
	    else
		lastcr = FALSE;		/* no longer following CR */
	
	}
	c &= 0177;			/* trim character */
    }
    Set1val(c);				/* set this as new value */
    return(XECCONT);
}
/* outctt -- output character for ^T command */

static void
outctt(c)
char c;					/* character to output */
{
    TNUMB oldEU = EU;			/* remember case flagging flag */

    EU = -1;				/* turn off case flagging */
    wCch(c, Cmdout);			/* write character */
    EU = oldEU;				/* restore casing flag */
    return;
}
/* ^L (FF) command
**
** Write form feed character to terminal.
*/

short
CMff()
{
    if (Skiptype == SKNONE)		/* if not skipping ... */
	wCch(FF, Cmdout);		/* write FF char */
    return(CONTINUE);
}
